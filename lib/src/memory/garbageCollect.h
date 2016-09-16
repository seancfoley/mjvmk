#ifndef GARBAGECOLLECT_H
#define GARBAGECOLLECT_H

#include "types/config.h"
#include "memoryAlloc.h"



#define BLACK_AND_ALLOCATED_MASK 0x1d /* 00011101 NON_COLLECTIBLE_BIT | ALL_COLLECTIBLE_BITS | BLACK_BIT */
#define REFERENCED_MASK 0x11          /* 00010001 NON_COLLECTIBLE_BIT | BLACK_BIT */
#define BLACK_MASK_1 0x0d             /* 00001101 */
#define BLACK_MASK_2 0x09             /* 00001001 */
#define BLACK_MASK_3 0x05             /* 00000101 */
#define GREY_MASK_1 0x0e              /* 00001110 */
#define GREY_MASK_2 0x0a              /* 00001010 */
#define GREY_MASK_3 0x06              /* 00000110 */
#define BLACK_GREY_MASK_1 0x0f        /* 00001111 */
#define BLACK_GREY_MASK_2 0x0b        /* 00001011 */
#define BLACK_GREY_MASK_3 0x07        /* 00000111 */
#define FREE_MASK 0x00                /* 00000000 */
#define SET_WHITE_MASK 0xfffffffc     /* 11111111 11111111 11111111 11111100 */


#define isBlack(pBlock) (((BLOCK_HEADER) (pBlock))->key & BLACK_BIT)
#define isGrey(pBlock) (((BLOCK_HEADER) (pBlock))->key & GREY_BIT)
#define setBlack(pBlock) (((BLOCK_HEADER) (pBlock))->key |= BLACK_BIT)
#define setGrey(pBlock) (((BLOCK_HEADER) (pBlock))->key |= GREY_BIT)
#define isReferenced(pBlock) (((BLOCK_HEADER) (pBlock))->key & REFERENCED_MASK)
#define isNotReferenced(pBlock) (!isReferenced(pBlock))
#define getBlackAndAllocatedFlagBits(pBlock) (((BLOCK_HEADER) (pBlock))->key & BLACK_AND_ALLOCATED_MASK)
#define setMaskWhite(mask) ((mask) &= SET_WHITE_MASK)
#define setWhite(pBlock) setMaskWhite(((BLOCK_HEADER) (pBlock))->key)

#define getContainedBlockCount(pBlock) ((pBlock)->forward.indices.numContainedBlocks)


SUCCESS_CODE garbageCollectForMemory(UINT32 size, UINT32 blockMask, UINT16 objectFieldCount, UINT32 locationByteOffset, void **ppLocation);
void garbageCollect();
void whiten();
void setGCMarkFunction(void (*markF)());



#endif
