#ifndef MEMORYALLOC_H
#define MEMORYALLOC_H


#include "heap.h"

/* for garbage collector use only */

void addToFreeList(FREE_LIST_BLOCK_HEADER pReleasedMemoryBlocks, FREE_LIST_BLOCK_HEADER pLastInReleasedList);
SUCCESS_CODE allocateBlock(UINT32 size, UINT32 blockMask, UINT16 numContainedItems, UINT32 locationByteOffset, void **ppLocation);

/* memory allocation API */

void *memoryAlloc(UINT32 numBytes);
void *memoryCalloc(UINT32 numBytes, UINT32 size);
void memoryFree(void *pMem);
SUCCESS_CODE collectibleMemoryAlloc(UINT32 byteSize, UINT32 blockMask, UINT16 numContainedItems, UINT32 locationByteOffset, void **ppLocation);
SUCCESS_CODE memoryCallocMonitor(MONITOR *ppMonitor);
SUCCESS_CODE resizeTable(void **table, UINT16 *currentTableSize, UINT32 entrySize, UINT16 blockSize, UINT16 requiredTableSize);

extern MONITOR pMonitorAllocationMonitor;
#define enterMonitorAllocationMonitor() monitorEnter(pMonitorAllocationMonitor)
#define exitMonitorAllocationMonitor() monitorExit(pMonitorAllocationMonitor)

/* memory info API */

UINT32 getTotalAvailableMemory();
UINT32 getTotalMemory();



#endif
