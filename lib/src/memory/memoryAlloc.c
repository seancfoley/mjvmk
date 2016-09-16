
#include "thread/schedule.h"
#include "heap.h"
#include "garbageCollect.h"
#include "memoryAlloc.h"

FREE_LIST_BLOCK_HEADER pFreeList;
void *pHeap;
void *pHeapEnd;


void addToFreeList(FREE_LIST_BLOCK_HEADER pReleasedMemoryBlocks, FREE_LIST_BLOCK_HEADER pLastInReleasedList)
{
    pLastInReleasedList->header.forward.pNextFree = pFreeList;
    if(pFreeList != NULL) {
        pFreeList->pPreviousFree = pLastInReleasedList;
    }
    pFreeList = pReleasedMemoryBlocks;
}


SUCCESS_CODE initializeHeap()
{
    SUCCESS_CODE ret = psInitializeHeap(&pHeap, HEAP_SIZE);
    if(ret != SC_SUCCESS) {
        return SC_FAILURE;
    }
    pHeapEnd = ((PBYTE) pHeap) + HEAP_SIZE;
    pFreeList = asFreeListBlockPtr(pHeap);
    pFreeList->pPreviousFree = NULL;
    pFreeList->header.forward.pNextFree = NULL;
    pFreeList->header.back.pPreviousContiguous = NULL;
    setSizeAndSetFree(pFreeList, HEAP_SIZE);
    return SC_SUCCESS;
}

monitorStruct heapAccessMonitor = {0, NULL, NULL, NULL};
MONITOR pHeapAccessMonitor = &heapAccessMonitor;

void memoryFree(void *pMem)
{
    FREE_LIST_BLOCK_HEADER pBlock = asFreeListBlockPtr(getBlockHeader(pMem));
    BLOCK_HEADER pNextContiguousBlock = pBlock->header.forward.pNextContiguous;
    BLOCK_HEADER pPreviousContiguousBlock = pBlock->header.back.pPreviousContiguous;
    BLOCK_HEADER pNextContiguousNeighbour;
    FREE_LIST_BLOCK_HEADER pPreviousFreeBlock;
    FREE_LIST_BLOCK_HEADER pNextFreeBlock;
    
    /* 
     * Only non-collectibles can be explicitly released.
     * Collectibles cannot be explicitly released, they must be garbage collected.
     */
    ASSERT(isAllocatedNonCollectible(pBlock));
    
    enterHeapAccessMonitor();
    if(pPreviousContiguousBlock != NULL && isFree(pPreviousContiguousBlock)) {
        if(pNextContiguousBlock != NULL && isFree(pNextContiguousBlock)) {
          
            /* join the three contiguous blocks together */
            
            /* update the free list */
            pPreviousFreeBlock = asFreeListBlockPtr(pNextContiguousBlock)->pPreviousFree;
            pNextFreeBlock = pNextContiguousBlock->forward.pNextFree;
            if(pPreviousFreeBlock != NULL) {
                pPreviousFreeBlock->header.forward.pNextFree = pNextFreeBlock;
            }
            else {
                pFreeList = pNextFreeBlock;
            }
            if(pNextFreeBlock != NULL) {
                pNextFreeBlock->pPreviousFree = pPreviousFreeBlock;
            }
            
            /* make sure the next contiguous block knows that we've joined together */
            pNextContiguousNeighbour = getFollowingBlock(pNextContiguousBlock);
            if(isNotPastHeapEnd(pNextContiguousNeighbour) && isAllocatedNonCollectible(pNextContiguousNeighbour)) {
                pNextContiguousNeighbour->back.pPreviousContiguous = pPreviousContiguousBlock;
            }

            setSizeAndSetFree(pPreviousContiguousBlock, 
                getSize(pPreviousContiguousBlock) + getSize(pBlock) + getSize(pNextContiguousBlock));
        }
        else {
            if(pNextContiguousBlock != NULL && isAllocatedNonCollectible(pNextContiguousBlock)) {
                pNextContiguousBlock->back.pPreviousContiguous = pPreviousContiguousBlock;
            }
            /* join the block with its previous contiguous block */
            setSizeAndSetFree(pPreviousContiguousBlock, 
                getSize(pPreviousContiguousBlock) + getSize(pBlock));
        }
    }
    /* note that there is no need to change the block key.  To the garbage collector it does
     * not matter if the block is seen as non-collectible or free, it will skip the block either way.
     * When the block is allocated next, the whole block key will be overwritten.
     */ 
    else if(pNextContiguousBlock != NULL  && isFree(pNextContiguousBlock)) {

        /* join the block with the next contiguous block */

        /* update the free list */
        pPreviousFreeBlock = asFreeListBlockPtr(pNextContiguousBlock)->pPreviousFree;
        pNextFreeBlock = pNextContiguousBlock->forward.pNextFree;
        if(pPreviousFreeBlock) {
            pPreviousFreeBlock->header.forward.pNextFree = pBlock;
        }
        else {
            pFreeList = pBlock;
        }
        pBlock->pPreviousFree = pPreviousFreeBlock;
        if(pNextFreeBlock != NULL) {
            pNextFreeBlock->pPreviousFree = pBlock;
        }
        pBlock->header.forward.pNextFree = pNextFreeBlock;
        
        /* the previous contiguous block is already set, no need to reset it */

        /* make sure the next contiguous block knows that we've joined together */
        pNextContiguousNeighbour = getFollowingBlock(pNextContiguousBlock);
        if(isNotPastHeapEnd(pNextContiguousNeighbour) && isAllocatedNonCollectible(pNextContiguousNeighbour)) {
            pNextContiguousNeighbour->back.pPreviousContiguous = asBlockPtr(pBlock);
        }

        setSizeAndSetFree(pBlock, getSize(pBlock) + getSize(pNextContiguousBlock)); 
    }
    else {
        /* both neighbours are allocated, we simply add the block to the free list */
        pBlock->header.forward.pNextFree = pFreeList;
        pBlock->pPreviousFree = NULL;
        if(pFreeList != NULL) { /* if pFreeList is NULL, we arrived just in time, there was no heap memory left */
            pFreeList->pPreviousFree = pBlock;
        }
        pFreeList = pBlock;
        setFree(pBlock);       
    }
    exitHeapAccessMonitor();
    return;
}

/* Allocates blocks from the heap memory.
 *
 * 'ppLocation' will point to the allocated block.  'size' indicates the required size not including
 * the block header.
 *
 * 'locationByteOffset' is the offset from the top of the block to where ppLocation will point.
 * For garbage collection purposes, 'numContainedItems' will indicate how many other memory blocks
 * can be referenced from this block.  Set to zero for non-collectible blocks.
 * 'blockMask' is the 32-bit key to be contained in the block header, which indicates whether the
 * block is collectible and what type of collectible block it is.
 */
SUCCESS_CODE allocateBlock(UINT32 size, UINT32 blockMask, UINT16 numContainedItems, UINT32 locationByteOffset, void **ppLocation)
{
    FREE_LIST_BLOCK_HEADER pCurrentBlock = pFreeList;
    FREE_LIST_BLOCK_HEADER *ppPreviousPtr = &pFreeList;
    FREE_LIST_BLOCK_HEADER pPreviousBlock = NULL;
    FREE_LIST_BLOCK_HEADER pNewBlock;
    FREE_LIST_BLOCK_HEADER pNextBlock;
    UINT32 requiredSize = size + getBlockHeaderSize();
    UINT32 extraSize;
    UINT32 blockSize;
    
    if(requiredSize < getMinimumBlockSize()) {
        requiredSize = getMinimumBlockSize();
    }

    
    while(pCurrentBlock != NULL) {
        blockSize = getSize(pCurrentBlock);

        
        if(blockSize >= requiredSize) {
            extraSize = blockSize - requiredSize;

            
            if(extraSize >= getMinimumBlockSize()) { /* big enough to split into two blocks */
                pNewBlock = pCurrentBlock;
                pCurrentBlock = getNextBlockAsFreeBlock(pCurrentBlock, extraSize);
                pNextBlock = getNextBlockAsFreeBlock(pCurrentBlock, requiredSize);
                
                /* an allocated non-collectible block must point to its contiguous neighbours */
                if(isAllocatedNonCollectibleMask(blockMask)) {
                    pCurrentBlock->header.back.pPreviousContiguous = asBlockPtr(pNewBlock);
                    if(isNotPastHeapEnd(pNextBlock)) {
                        pCurrentBlock->header.forward.pNextContiguous = asBlockPtr(pNextBlock);
                    }
                    else {
                        pCurrentBlock->header.forward.pNextContiguous = NULL;
                    }
                }
                else { /* this block will be collectible */
                    concurrentGCBlackenMask(blockMask);
                    
                    /* this newly allocated block must be seen as referenced by the garbage collector's current cycle */
                    pCurrentBlock->header.forward.indices.numContainedBlocks = numContainedItems;
                }
                
                /* update the next contiguous allocated non-collectible block to point to its new neighbour */
                if(isNotPastHeapEnd(pNextBlock) && isAllocatedNonCollectible(pNextBlock)) {
                    pNextBlock->header.back.pPreviousContiguous = asBlockPtr(pCurrentBlock);
                }

                
                /* set the size of the new block */
                setSizeAndSetFree(pCurrentBlock, requiredSize);

                /* put the block in its new location before it can be seen as collectible */
                *ppLocation = getBlockOffset(pCurrentBlock, getBlockHeaderSize()) + locationByteOffset;
                
                /* finish off the header by setting the mask */
                setKeyFlags(pCurrentBlock, blockMask);

                /* now that the header is complete, it can be identified as a block by changing the
                 * length of the block being split up.
                 */
                setSizeAndSetFree(pNewBlock, extraSize);

                
            }
            else { /* remove this block from the free list */

                /* update the free list */
                *ppPreviousPtr = pNextBlock = pCurrentBlock->header.forward.pNextFree;
                if(pNextBlock) {
                    pNextBlock->pPreviousFree = pPreviousBlock;
                }
                
                /* an allocated non-collectible block must point to its contiguous neighbours */
                if(isAllocatedNonCollectibleMask(blockMask)) {
                    /* the pointer to the previous contiguous block is already set */
                    pNextBlock = getNextBlockAsFreeBlock(pCurrentBlock, blockSize);
                    if(isPastHeapEnd(pNextBlock)) {
                        pCurrentBlock->header.forward.pNextContiguous = NULL;
                    }
                    else {
                        pCurrentBlock->header.forward.pNextContiguous = asBlockPtr(pNextBlock);
                    }
                }
                else {
                    concurrentGCBlackenMask(blockMask);
                    
                    /* this newly allocated block must be seen as referenced by the garbage collector's current cycle */
                    pCurrentBlock->header.forward.indices.numContainedBlocks = numContainedItems;
                }
                
                /* put the block in its new location before it can be seen as collectible */
                *ppLocation = getBlockOffset(pCurrentBlock, getBlockHeaderSize()) + locationByteOffset;

                /* set the flags - if collectible from this point onwards the garbage collector will look at this block */
                setKeyFlags(pCurrentBlock, blockMask);
            }

            
            if(pCurrentBlock == (struct freeListBlockHeader *) 0x9f3318) {
                int a = 4;
            }

            return SC_SUCCESS;
        }

        /* try the next free block */
        pPreviousBlock = pCurrentBlock;
        ppPreviousPtr = &pCurrentBlock->header.forward.pNextFree;
        pCurrentBlock = *ppPreviousPtr;
    }

    *ppLocation = NULL;
    return SC_FAILURE;
}

/* equivalent to malloc */
void *memoryAlloc(UINT32 numBytes)
{
    BLOCK_HEADER pMem;
    SUCCESS_CODE ret;

    enterHeapAccessMonitor();
    ret = allocateBlock(numBytes, NON_COLLECTIBLE_BIT, 0, 0, &pMem);
    exitHeapAccessMonitor();
    if(ret != SC_SUCCESS) {
        garbageCollectForMemory(numBytes, NON_COLLECTIBLE_BIT, 0, 0, &pMem);
    }
    return pMem;
}

/* equivalent to calloc */
void *memoryCalloc(UINT32 num, UINT32 size)
{
    UINT32 numBytes = num * size;
    BLOCK_HEADER pMem;
    SUCCESS_CODE ret;

    enterHeapAccessMonitor();
    ret = allocateBlock(numBytes, NON_COLLECTIBLE_BIT, 0, 0, &pMem);
    exitHeapAccessMonitor();
    
    if(ret != SC_SUCCESS) {
        ret = garbageCollectForMemory(numBytes, NON_COLLECTIBLE_BIT, 0, 0, &pMem);
        if(ret == SC_SUCCESS) {
            memorySet(pMem, 0, numBytes);
        }
        else {
            pMem = NULL;
        }
    }
    else {
        memorySet(pMem, 0, numBytes);
    }
    return pMem;
}


/* this monitor prevents two threads from allocating the same monitor simultaneously */
monitorStruct monitorAllocationMonitor = {0, NULL, NULL, NULL};
MONITOR pMonitorAllocationMonitor = &monitorAllocationMonitor;

/* use this function to allocate a monitor if more than one thread might be checking
 * the same location for its existence, so that the monitor is not allocated more than once.
 */
SUCCESS_CODE memoryCallocMonitor(MONITOR *ppMonitor) 
{
    SUCCESS_CODE ret;
    
    enterMonitorAllocationMonitor();
    if(*ppMonitor) {
        exitMonitorAllocationMonitor();
        return SC_SUCCESS;
    }

    enterHeapAccessMonitor();
    ret = allocateBlock(sizeof(monitorStruct), NON_COLLECTIBLE_BIT, 0, 0, ppMonitor);
    exitHeapAccessMonitor();
    
    if(ret != SC_SUCCESS) {
        ret = garbageCollectForMemory(sizeof(monitorStruct), NON_COLLECTIBLE_BIT, 0, 0, ppMonitor);
        if(ret == SC_SUCCESS) {
            monitorInit(*ppMonitor);
        }
    }
    else {
        monitorInit(*ppMonitor);
    }
    exitMonitorAllocationMonitor();
    return ret;
    
}

/* collectible memory equivalent to malloc */
SUCCESS_CODE collectibleMemoryAlloc(UINT32 byteSize, UINT32 blockMask, UINT16 numContainedItems, UINT32 locationByteOffset, void **ppLocation)
{
    SUCCESS_CODE ret;

    enterHeapAccessMonitor();
    ret = allocateBlock(byteSize, blockMask, numContainedItems, locationByteOffset, ppLocation);
    exitHeapAccessMonitor();

    if(ret != SC_SUCCESS) {
        ret = garbageCollectForMemory(byteSize, blockMask, numContainedItems, locationByteOffset, ppLocation);
    }
    return ret;
}


SUCCESS_CODE resizeTable(void **table, UINT16 *currentTableSize, UINT32 entrySize, UINT16 blockSize, UINT16 requiredTableSize)
{
    void *newTable;
    void *oldTable = *table;
    UINT16 newTableSize = *currentTableSize;
    
    while(requiredTableSize > newTableSize) {
        newTableSize += blockSize;
    }

    newTable = memoryAlloc(entrySize * newTableSize);
    if(newTable == NULL) {
        return SC_FAILURE;
    }
    
    if(oldTable != NULL) {
        memoryCopy(newTable, oldTable, entrySize * (*currentTableSize));
        memoryFree(oldTable);
    }
    *currentTableSize = newTableSize;
    *table = newTable;
    return SC_SUCCESS;
}


UINT32 getTotalMemory()
{
    return ((PBYTE) pHeapEnd) - (PBYTE) pHeap;
}

UINT32 getTotalAvailableMemory()
{
    FREE_LIST_BLOCK_HEADER pAvailableBlock = pFreeList;
    UINT32 total = 0;

    while(pAvailableBlock != NULL) {
        total += getSize(pAvailableBlock);
        pAvailableBlock = pAvailableBlock->header.forward.pNextFree;
    }
    return total;
}



