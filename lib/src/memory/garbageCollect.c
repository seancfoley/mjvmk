
#include "garbageCollect.h"



/* returns a pointer to the list of released memory blocks, and also 
 * passes a pointer to the last entry in the same list.
 */
static FREE_LIST_BLOCK_HEADER sweep(FREE_LIST_BLOCK_HEADER *ppLastInReleasedList)
{
    FREE_LIST_BLOCK_HEADER pCollectibleBlock = pHeap;
    BLOCK_HEADER pPreviousContiguous = NULL;
    BLOCK_HEADER pNonCollectibleBlock;
    FREE_LIST_BLOCK_HEADER pFreeBlock;
    FREE_LIST_BLOCK_HEADER pReleasedMemoryBlocks = NULL;
    
checkNextBlock:
    
    switch(getBlackAndAllocatedFlagBits(pCollectibleBlock)) {

        case NON_COLLECTIBLE_BIT:   /* non-collectible */
        case REFERENCED_MASK:       /* non-collectible and black */
		case BLACK_MASK_1:
		case BLACK_MASK_2:
		case BLACK_MASK_3:
		case FREE_MASK: /* the block is free */
            pPreviousContiguous = asBlockPtr(pCollectibleBlock);
            pCollectibleBlock = getFollowingBlockAsFreeBlock(pCollectibleBlock);
            if(isPastHeapEnd(pCollectibleBlock)) {
                return pReleasedMemoryBlocks;
            }
            goto checkNextBlock;

        /* default:
         *
         * ERROR conditions that should never occur:
         * a block is marked both collectible and not collectible: 0x1c, 0x1d, 0x18, 0x19, 0x14, 0x15
         * a block is marked free and black: BLACK_BIT 
         *
         *
         * the block can be collected: 
         * COLLECTIBLE_BIT_1, 
         * COLLECTIBLE_BIT_2, 
         * ALL_COLLECTIBLE_BITS 
         */
    }

    /* pCollectibleBlock is now pointing to a collectible block */

    pNonCollectibleBlock = getFollowingBlock(pCollectibleBlock);

    /* keep going until we find a non-free block that is not collectible */

    if(isNotPastHeapEnd(pNonCollectibleBlock) && isNotReferenced(pNonCollectibleBlock)) {
        do {
            if(isFree(pNonCollectibleBlock)) { /* free blocks must be removed from the free list */
                pFreeBlock = asFreeListBlockPtr(pNonCollectibleBlock);
                if(pFreeBlock->pPreviousFree != NULL) {
                    pFreeBlock->pPreviousFree->header.forward.pNextFree = pFreeBlock->header.forward.pNextFree;
                }
                else {
                    pFreeList = pFreeBlock->header.forward.pNextFree;
                }
                if(pFreeBlock->header.forward.pNextFree) {
                    pFreeBlock->header.forward.pNextFree->pPreviousFree = pFreeBlock->pPreviousFree;
                }
            }
            pNonCollectibleBlock = getFollowingBlock(pNonCollectibleBlock);
        } while(isNotPastHeapEnd(pNonCollectibleBlock) && isNotReferenced(pNonCollectibleBlock));

        if(pPreviousContiguous != NULL && isFree(pPreviousContiguous)) {

            /* all we need to do is join this block to the previous free block */
            setSizeAndSetFree(pPreviousContiguous, getSize(pPreviousContiguous) + (((PBYTE) pNonCollectibleBlock) - (PBYTE) pCollectibleBlock));

            /* ensure the next contiguous block knows we have changed size */
            if(isNotPastHeapEnd(pNonCollectibleBlock) && isAllocatedNonCollectible(pNonCollectibleBlock)) {
                pNonCollectibleBlock->back.pPreviousContiguous = pPreviousContiguous;
            }
            if(isNotPastHeapEnd(pNonCollectibleBlock)) {
                pCollectibleBlock = getFollowingBlockAsFreeBlock(pNonCollectibleBlock);
                if(isNotPastHeapEnd(pCollectibleBlock)) {
                    pPreviousContiguous = pNonCollectibleBlock;
                    goto checkNextBlock;
                }
            }
            return pReleasedMemoryBlocks;

        }
        else {
            setSizeAndSetFree(pCollectibleBlock, (((PBYTE) pNonCollectibleBlock) - (PBYTE) pCollectibleBlock));

            /* ensure the next contiguous block knows we have changed size */
            if(isNotPastHeapEnd(pNonCollectibleBlock) && isAllocatedNonCollectible(pNonCollectibleBlock)) {
                pNonCollectibleBlock->back.pPreviousContiguous = asBlockPtr(pCollectibleBlock);
            }

        }

    }
    else if(pPreviousContiguous != NULL && isFree(pPreviousContiguous)) {

        /* all we need to do is join this block to the previous free block */
        setSizeAndSetFree(pPreviousContiguous, getSize(pPreviousContiguous) + getSize(pCollectibleBlock));
        
        if(isNotPastHeapEnd(pNonCollectibleBlock)) {
            /* ensure the next contiguous block knows we have changed size */
            if(isAllocatedNonCollectible(pNonCollectibleBlock)) {
                pNonCollectibleBlock->back.pPreviousContiguous = pPreviousContiguous;
            }
            pCollectibleBlock = getFollowingBlockAsFreeBlock(pNonCollectibleBlock);
            if(isNotPastHeapEnd(pCollectibleBlock)) {
                pPreviousContiguous = pNonCollectibleBlock;
                goto checkNextBlock;
            }
        }
        return pReleasedMemoryBlocks;
    }
    else {
        setFree(pCollectibleBlock);
    }

    /* add the block to the list of released blocks, which will eventually be added to the free list */

    pCollectibleBlock->header.forward.pNextFree = pReleasedMemoryBlocks;
    pCollectibleBlock->header.back.pPreviousContiguous = pPreviousContiguous;
    pCollectibleBlock->pPreviousFree = NULL;
    if(pReleasedMemoryBlocks != NULL) {
        pReleasedMemoryBlocks->pPreviousFree = pCollectibleBlock;
    }
    else {
        *ppLastInReleasedList = pCollectibleBlock;
    }
    pReleasedMemoryBlocks = pCollectibleBlock;


    if(isNotPastHeapEnd(pNonCollectibleBlock)) {
        pCollectibleBlock = getFollowingBlockAsFreeBlock(pNonCollectibleBlock);
        if(isNotPastHeapEnd(pCollectibleBlock)) {
            pPreviousContiguous = pNonCollectibleBlock;
            goto checkNextBlock;
        }
    }
    return pReleasedMemoryBlocks;
}

/* marks all collectible blocks as white, and hence garbage collectible */
void whiten()
{
    BLOCK_HEADER pBlock = asBlockPtr(pHeap);

    while(isNotPastHeapEnd(pBlock)) {
        setWhite(pBlock);
        pBlock = getFollowingBlock(pBlock);
    }
}


static FREE_LIST_BLOCK_HEADER runGarbageCollection(FREE_LIST_BLOCK_HEADER *ppLastInReleasedList, void (*markFunc)())
{
    FREE_LIST_BLOCK_HEADER pReleasedMemoryBlocks;

#if !CONCURRENT_GC
    disableRescheduler();
#endif

    markFunc();
    pReleasedMemoryBlocks = sweep(ppLastInReleasedList);

#if !CONCURRENT_GC
    enableRescheduler();
#endif
    
    return pReleasedMemoryBlocks;
}


/* this monitor prevents simultaneous garbage collections */
monitorStruct garbageCollectionWaitMonitor = {0, NULL, NULL, NULL};
MONITOR pGarbageCollectionWaitMonitor = &garbageCollectionWaitMonitor;
BOOLEAN garbageCollectionInProgress;

/* If this function is used as the garbage collector mark function, then nothing
 * will be marked, so everything will be collected.
 */
static void nullMarker()
{
	return;
}

void (*markFunc)() = nullMarker;

/* calling with an argument of NULL will cause all memory to be garbage collected 
 */
void setGCMarkFunction(void (*markF)())
{
	if(markF == NULL) {
		markFunc = nullMarker;
	}
	else {
		markFunc = markF;
	}
}

/* we are not short on memory, we just wish to collect garbage */
void garbageCollect()
{
    FREE_LIST_BLOCK_HEADER pReleasedMemoryBlocks;
    FREE_LIST_BLOCK_HEADER pLastInReleasedList;

    monitorEnter(pGarbageCollectionWaitMonitor);
    if(!garbageCollectionInProgress) {
        garbageCollectionInProgress = TRUE;
        monitorExit(pGarbageCollectionWaitMonitor);
        
        pReleasedMemoryBlocks = runGarbageCollection(&pLastInReleasedList, markFunc);
        
        monitorEnter(pGarbageCollectionWaitMonitor);
        garbageCollectionInProgress = FALSE;
        enterHeapAccessMonitor();
        if(pReleasedMemoryBlocks != NULL) {
            addToFreeList(pReleasedMemoryBlocks, pLastInReleasedList);
        }
        exitHeapAccessMonitor();
    }
    monitorExit(pGarbageCollectionWaitMonitor);
}


/* we are collecting garbage because we need a memory block */
SUCCESS_CODE garbageCollectForMemory(UINT32 size, UINT32 blockMask, UINT16 objectFieldCount, UINT32 locationByteOffset, void **ppLocation)
{
    SUCCESS_CODE ret;
    FREE_LIST_BLOCK_HEADER pReleasedMemoryBlocks;
    FREE_LIST_BLOCK_HEADER pLastInReleasedList;

    monitorEnter(pGarbageCollectionWaitMonitor);
    do {
        if(!garbageCollectionInProgress) {
            garbageCollectionInProgress = TRUE;
            monitorExit(pGarbageCollectionWaitMonitor);
            pReleasedMemoryBlocks = runGarbageCollection(&pLastInReleasedList, markFunc);

            /* try to acquire the required memory block */

            enterHeapAccessMonitor();
            if(pReleasedMemoryBlocks != NULL) {
                addToFreeList(pReleasedMemoryBlocks, pLastInReleasedList);
            }
            ret = allocateBlock(size, blockMask, objectFieldCount, locationByteOffset, ppLocation);
            exitHeapAccessMonitor();

            monitorEnter(pGarbageCollectionWaitMonitor);
            garbageCollectionInProgress = FALSE;
            monitorNotify(pGarbageCollectionWaitMonitor);

            /* if we failed to allocate the required block, then there is no use trying
             * again, because we had the first stab at any released memory.
             */
            break;
        }
        else {
            monitorWait(pGarbageCollectionWaitMonitor, NULL);
        
            /* try to acquire the required memory block */
            enterHeapAccessMonitor();
            ret = allocateBlock(size, blockMask, objectFieldCount, locationByteOffset, ppLocation);
            exitHeapAccessMonitor();

            /* if we failed to get the required block, it may be because the
             * memory released by garbage collection was consumed by other threads,
             * particularly the thread that performed the garbage collection.
             * So we try another garbage collection.
             *
             * Also, the lack of memory available to another thread might cause its death,
             * thus potentially releasing memory to this thread, so we should always try another
             * garbage collection before we give up.
             */
            if(ret == SC_SUCCESS) {
                break;
            }
        }
    } while(TRUE);
    monitorExit(pGarbageCollectionWaitMonitor);
    return ret;
}

