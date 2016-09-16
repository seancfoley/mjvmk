
#include "memory/memoryAlloc.h"
#include "log/logItem.h"
#include "stackSegment.h"



#if GROW_STACK


RETURN_CODE allocateStack(UINT32 requiredSpace, STACK_SEGMENT pCurrentSegment, STACK_PARAMS pStackParams)
{
    UINT32 requiredStackLength = JAVA_STACK_BLOCK_SIZE;
    STACK_SEGMENT pNextSegment;
    STACK_SEGMENT pNewSegment; 
    
    while(requiredStackLength < requiredSpace) {
        requiredStackLength += JAVA_STACK_BLOCK_SIZE;
    }
    pNextSegment = pCurrentSegment->pNext;

    if(pNextSegment == NULL || pNextSegment->length < requiredStackLength) {
        
        JSTACK_FIELD pField = memoryAlloc((getStackFieldSize() * requiredStackLength) + sizeof(stackSegmentStruct));
        if(pField == NULL) {
            return ERROR_CODE_OUT_OF_MEMORY;
        }
        pNewSegment = pCurrentSegment->pNext = (STACK_SEGMENT) (pField + requiredStackLength);
        pStackParams->pBase = ((JSTACK_FIELD) pNewSegment) - requiredStackLength;
        pNewSegment->length = requiredStackLength;

        /* we could set pNext to NULL here and release the memory of a non-NULL pNextSegment 
         * but instead we choose to save it for later
         */
        pNewSegment->pNext = pNextSegment;

        LOG_LINE(("allocated new stack segment %x", pNewSegment));
    }
    else {
        pStackParams->pBase = ((JSTACK_FIELD) pNextSegment) - pNextSegment->length;
        pNewSegment = pNextSegment;

        LOG_LINE(("reusing previously allocated stack segment %x", pNewSegment));
    }
    pStackParams->pLimit = ((JSTACK_FIELD) pNewSegment) - 1;
    return SUCCESS;
}

/* sp points to where the new frame would normally go */
/* returns the base of the new stack */
JSTACK_FIELD startNewStackSegment(METHOD_DEF pMethodDef, JSTACK_FIELD sp, JSTACK_FIELD pCurrentStackLimit, JSTACK_FIELD *ppNewStackLimit)
{
    stackParamStruct pStackParams;
    RETURN_CODE ret;
    UINT16 parameterCount = pMethodDef->base.parameterVarCount;

    /* returns a pointer to the new parameter location */
    ret = allocateStack(pMethodDef->invokeCount, getCurrentSegment(pCurrentStackLimit), &pStackParams);
    if(ret != SUCCESS) {
        return NULL;
    }
    *ppNewStackLimit = pStackParams.pLimit;
    memoryCopy(pStackParams.pBase, sp - parameterCount, parameterCount * getStackFieldSize());
    return pStackParams.pBase;
}

#endif


RETURN_CODE allocateStackBase(STACK_SEGMENT *ppStack, STACK_PARAMS pStackParams, UINT32 initialStackSize)
{
    STACK_SEGMENT pStack;
    JSTACK_FIELD pField;

    pStackParams->pBase = pField = memoryAlloc((getStackFieldSize() * initialStackSize) + sizeof(stackSegmentStruct));
    if(pField == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    *ppStack = pStack = (STACK_SEGMENT) (pField + initialStackSize);
    pStack->length = initialStackSize;
    pStack->pNext = NULL;
    pStackParams->pLimit = ((JSTACK_FIELD) pStack) - 1;
    return SUCCESS;
}

void deAllocateStack(STACK_SEGMENT *ppStackBase)
{
    STACK_SEGMENT pNextSegment;
    STACK_SEGMENT pCurrentSegment = *ppStackBase;

    do {
        pNextSegment = pCurrentSegment->pNext;
        memoryFree(((JSTACK_FIELD) pCurrentSegment) - pCurrentSegment->length);
        pCurrentSegment = pNextSegment;
    } while(pCurrentSegment != NULL);
    *ppStackBase = NULL;
    return;
}


