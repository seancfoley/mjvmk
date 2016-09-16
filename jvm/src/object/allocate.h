#ifndef ALLOCATE_H
#define ALLOCATE_H

#include "elements/base.h"
#include "interpret/returnCodes.h"

/*
 * The following indicates how the collectible bits in the marking process are used (see heap.h):
 * (010) - collectible object array with length > 0
 * (001) - collectible object with object fields
 * (011) - any other type of collectible memory block, 
 *          such as objects with only primitive fields or no fields at all, zero length object arrays, 
 *          primitive arrays, and collectible monitors
 *
 * Some structures such as monitors can be allocated as either collectible or not collectible.
 * In such blocks, the garbage collector might read or use the black and grey bits even if the block
 * is not collectible.  In particular, java.lang.Class objects, which are non-collectible, can be marked grey 
 * if found in a static field or black if found in an object field.  
 * The garbage collector will never collect any such block during the sweep phase, no matter which gc bits are set.
 *
 */ 

#define REGULAR_COLLECTIBLE_BITS ALL_COLLECTIBLE_BITS /* both above bits set indicate some other type of allocated and collectible block */                     
#define OBJECT_WITH_OBJECT_FIELDS_BIT COLLECTIBLE_BIT_1 /* indicates an allocated object with object fields */
#define OBJECT_ARRAY_WITH_FIELDS_BIT COLLECTIBLE_BIT_2 /* indicates an allocated non-zero size object array object */

#define GREYED_SIMPLE_OBJECT_MASK GREY_MASK_1 /* GREY_BIT | ALL_COLLECTIBLE_BITS */
#define GREYED_ARRAY_FIELDS_MASK GREY_MASK_2 /* GREY_BIT | OBJECT_ARRAY_WITH_FIELDS_BIT */
#define GREYED_OBJECT_FIELDS_MASK GREY_MASK_3 /* GREY_BIT | OBJECT_WITH_OBJECT_FIELDS_BIT */

#define BLACK_OBJECT_FIELDS_MASK BLACK_MASK_3 /* BLACK_BIT | OBJECT_WITH_OBJECT_FIELDS_BIT */
#define BLACK_ARRAY_FIELDS_MASK BLACK_MASK_2 /* BLACK_BIT | OBJECT_ARRAY_WITH_FIELDS_BIT */
#define BLACK_GREY_ARRAY_FIELDS_MASK BLACK_GREY_MASK_2 /* GREY_BIT | BLACK_BIT | OBJECT_ARRAY_WITH_FIELDS_BIT */
#define BLACK_GREY_OBJECT_FIELDS_MASK BLACK_GREY_MASK_3 /* GREY_BIT | BLACK_BIT | OBJECT_WITH_OBJECT_FIELDS_BIT */

#define getObjectHeader(pObject, objectFieldCount) (asBlockPtr(getObjectField(pObject, objectFieldCount - 1)) - 1)
#define getContainedObjectCount(pBlock) getContainedBlockCount(pBlock)

#if PREEMPTIVE_THREADS && CONCURRENT_GC
#define concurrentGCGreyObject(pObject) {if(pObject) { setGrey(getObjectHeader(pObject, getObjectFieldCount(pObject))); }}
#else
#define concurrentGCGreyObject(pObject)
#endif



RETURN_CODE collectibleMemoryCallocObject(UINT32 byteSize, UINT16 objectFieldCount, OBJECT_INSTANCE *ppInstance);
RETURN_CODE collectibleMemoryCallocArray(UINT32 byteSize, UINT16 objectFieldCount, ARRAY_INSTANCE *ppInstance);
RETURN_CODE collectibleMemoryCallocMonitor(MONITOR *ppMonitor);

void javaMarkFunction();


#if LOGGING

void logHeap();
#define COLLECTOR_LOG(x) LOG_LINE(x)

#else

#define logHeap()
#define COLLECTOR_LOG(x)

#endif



#endif
