#ifndef HEAP_H
#define HEAP_H


#include "thread/schedule.h"
#include "types/config.h"
#include "psTypes.h"


/* Heap API */


/* initialization */

SUCCESS_CODE initializeHeap();


/* 
 * Every block in memory, whether allocated or free, collectible or not, 
 * has a 32-bit key of the following form:
 *
 * [ 24 bits for block size | 3 unused flag bits | 1 non-collectible bit | 2 collectible type bits | 2 gc bits ]
 *
 * The 3 non-collectible/collectible bits indicate the following:
 * (000) - the block is free
 * (100) - the block is allocated but not collectible
 * (0xx) - the block is collectible; the two bits allow for the identification of three collectible block types
 *
 * 
 * The 2 garbage collector bits indicate the following:
 * (00) - white block: it is not referenced from anywhere
 * (10) - grey block: the block is referenced, but has containing objects whose blocks have not yet been set grey or black
 * (11) or (01) - black block: the block is referenced and any object it references is also grey or black
 *
 */

typedef UINT32 blockHeaderKey;


#define NUM_FLAG_BITS 8 /* total number of flag bits in the key, the rest of the key is used for the size */
#define NON_COLLECTIBLE_BIT 0x10   /* 00010000 indicates a block is not to be collected */
#define COLLECTIBLE_BIT_1 0x04     /* 00000100 */
#define COLLECTIBLE_BIT_2 0x08     /* 00001000 */
#define ALL_COLLECTIBLE_BITS 0x0c  /* 00001100 */
#define GREY_BIT 0x02              /* 00000010 */
#define BLACK_BIT 0x01             /* 00000001 */



/* masks used to read or alter the above bits in ithe 32 bit key*/

#define FLAGS_MASK 0x000000ff      /* 00000000 00000000 00000000 11111111 */
#define SET_FREE_MASK 0xffffff00   /* 11111111 11111111 11111111 00000000 */
#define NOT_FREE_MASK 0x0000001c   /* 00000000 00000000 00000000 00011100 NON_COLLECTIBLE_BIT | ALL_COLLECTIBLE_BITS */



typedef struct blockHeader blockHeader, *BLOCK_HEADER;
typedef struct freeListBlockHeader freeListBlockHeader, *FREE_LIST_BLOCK_HEADER;


/* Every block of memory begins with this header.
 * Code clarity is the reason for declaring unions of pointers.
 */
struct blockHeader {
    blockHeaderKey key; /* a bitmap of information about the block as described above */

    union {
        BLOCK_HEADER pPreviousContiguous;       /* unallocated or allocated non-collectible: the previous contiguous block */
        BLOCK_HEADER pContainingObjectBlock;    /* allocated collectible: containing object during marking */
    } back;

    union {
        FREE_LIST_BLOCK_HEADER pNextFree;       /* unallocated: next in the free list */
        BLOCK_HEADER pNextContiguous;           /* allocated non-collectible: the next contiguous block */

        struct collectibleObjectFieldIndices {  /* allocated collectible: indices used during marking */
            UINT16 currentIndex;                /* the current containing block index in the marking process */
            UINT16 numContainedBlocks;          /* the total number of contained blocks */
        } indices;
    } forward;
};


#define getBlockHeaderSize() sizeof(blockHeader)



/* Items in the free list contain the block header as well as an extra pointer. 
 * The size of this structure is the minimum size of a memory block, because every memory
 * block at some point in time is free.
 */
struct freeListBlockHeader {
    blockHeader header;
    FREE_LIST_BLOCK_HEADER pPreviousFree;       /* the previous free block in the free list */
};

#define getFreeBlockHeaderSize() sizeof(freeListBlockHeader)
#define getMinimumBlockSize() getFreeBlockHeaderSize()


#define getSize(pBlock) (((BLOCK_HEADER) (pBlock))->key >> NUM_FLAG_BITS) /* the size indicates the total size of the block including the header */
#define getFlags(pBlock) (((BLOCK_HEADER) (pBlock))->key & FLAGS_MASK)
#define getAllocFlags(pBlock) (((BLOCK_HEADER) (pBlock))->key & NOT_FREE_MASK)
#define setSizeAndSetFree(pBlock, size) (FIRST_ASSERT((size) > 0) (((BLOCK_HEADER) (pBlock))->key = ((size) << NUM_FLAG_BITS)))
#define setKeyFlags(pBlock, mask) (((BLOCK_HEADER) (pBlock))->key |= mask) /* preserves any flags that were previously set */
#define setFree(pBlock) (((BLOCK_HEADER) (pBlock))->key &= SET_FREE_MASK)
#define isAllocatedNonCollectibleMask(mask) ((mask) & NON_COLLECTIBLE_BIT)
#define isAllocatedNonCollectible(pBlock) isAllocatedNonCollectibleMask(((BLOCK_HEADER) (pBlock))->key)
#define isFree(pBlock) (!(((BLOCK_HEADER) (pBlock))->key & NOT_FREE_MASK))

#if PREEMPTIVE_THREADS && CONCURRENT_GC
#define concurrentGCBlackenMask(mask) (mask |= BLACK_BIT)
#else
#define concurrentGCBlackenMask(mask)
#endif


#define asBlockPtr(ptr) ((BLOCK_HEADER) (ptr))
#define asFreeListBlockPtr(ptr) ((FREE_LIST_BLOCK_HEADER) (ptr))
#define getBlockOffset(pBlock, offset) (((PBYTE) pBlock) + offset)
#define getBlockData(pBlock) ((void *) getBlockOffset(pBlock, getBlockHeaderSize()))
#define getBlockHeader(pData) (asBlockPtr(pData) - 1)






extern FREE_LIST_BLOCK_HEADER pFreeList;
extern void *pHeap;
extern void *pHeapEnd;


#define getNextBlock(pBlock, size) (FIRST_ASSERT((size) > 0) ((BLOCK_HEADER) getBlockOffset(pBlock, size)))
#define getNextBlockAsFreeBlock(pBlock, size) (FIRST_ASSERT((size) > 0) ((FREE_LIST_BLOCK_HEADER) getBlockOffset(pBlock, size)))
#define getFollowingBlock(pBlock) getNextBlock(pBlock, getSize(pBlock))
#define getFollowingBlockAsFreeBlock(pBlock) getNextBlockAsFreeBlock(pBlock, getSize(pBlock))
#define isNotPastHeapEnd(pBlock) (asBlockPtr(pBlock) < asBlockPtr(pHeapEnd))
#define isPastHeapEnd(pBlock) (asBlockPtr(pBlock) >= asBlockPtr(pHeapEnd))



/* certain heap operations require uninterrupted access */

#if PREEMPTIVE_THREADS

extern MONITOR pHeapAccessMonitor;
#define enterHeapAccessMonitor() monitorEnter(pHeapAccessMonitor)
#define exitHeapAccessMonitor() monitorExit(pHeapAccessMonitor)

#else

#define enterHeapAccessMonitor()
#define exitHeapAccessMonitor()

#endif



#endif