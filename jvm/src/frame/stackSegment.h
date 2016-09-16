
#ifndef STACKSEGMENT_H
#define STACKSEGMENT_H

#include "elements/base.h"
#include "interpret/returnCodes.h"
#include "javaThread/javaThread.h"


/* a threads stack is divided into segments (just one if stack growth is not configured) */
struct stackSegmentStruct {
    STACK_SEGMENT pNext; 
    UINT32 length;
};

/* points to the stack boundaries within a stack chunk */
typedef struct stackParamStruct {
    JSTACK_FIELD pBase;  /* beginning of available stack */
    JSTACK_FIELD pLimit; /* end of available stack */
} stackParamStruct, *STACK_PARAMS;


/* the stack appears before the stackSegmentStruct, the stack limit is the begining of the stackSegmentStruct */
#define getCurrentSegment(pStackLimit) ((STACK_SEGMENT) (pStackLimit + 1))                              
#define stackAllocationIsRequired(sp, pStackLimit, required) ((sp) + (required) > (pStackLimit)) 

RETURN_CODE allocateStackBase(STACK_SEGMENT *ppStack, STACK_PARAMS pStackParams, UINT32 initialStackSize);
void deAllocateStack(STACK_SEGMENT *ppStackBase);

#if GROW_STACK
RETURN_CODE allocateStack(UINT32 requiredSize, STACK_SEGMENT pCurrentChunk, STACK_PARAMS pStackParams);
JSTACK_FIELD startNewStackSegment(METHOD_DEF pMethodDef, JSTACK_FIELD sp, JSTACK_FIELD pCurrentStackLimit, JSTACK_FIELD *ppNewStackLimit);
#endif

#endif

