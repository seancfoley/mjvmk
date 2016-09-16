

#include "elements/base.h"
#include "memory/garbageCollect.h"
#include "operate/log.h"
#include "interpret/instructions.h"
#include "interpret/javaGlobals.h"
#include "javaThread/javaThread.h"
#include "allocate.h"








/* Concurrent collection requires the following:
 *
 * Whenever an object is moved from a location1 to location2,
 * if location2 is searched earlier on in the gc marking process,
 * then the object is greyed so that if it is missed during marking,
 * it is ultimately not collected.
 *
 * The following are all instructions that move objects:
 * aaload, aastore, aload, aload_x, astore, astore_x, athrow, anewarray, getfield, putfield,
 * getstatic, putstatic, new, newarray, multianewarray, the 6 dups, swap, areturn,
 * and the associated quick instructions.
 * 
 * Also, objects can be moved from the pObjectToThrow member of each thread structure onto the stack.
 * 
 * Since threads are deactivated while their stack is searched for objects, the only operations above that
 * can cause objects to be missed while marking are:
 *
 * all the new operations: new, newarray, multianewarray, anewarray and their associated quick intructions
 * aaload (moved from an object to stack)
 * getfield and getfield_quick_object (moved from an object to stack)
 * putstatic and putstatic_quick_object (moved from stack to static field in class table)
 *
 * The above instructions can cause objects to be missed because of the order in which
 * the marking is done: class table static fields, then thread stacks, 
 * and then fields and array elements of previously markedobjects
 * 
 */



#define setObjectMonitorBlack(pMonitorVar, pObject) {       \
    pMonitorVar = getObjectMonitor(pObject);                \
    if(pMonitorVar != NULL) {                               \
        setBlack(getBlockHeader(pMonitorVar));              \
    }                                                       \
}



#define scanObject(pObject) {                                                                           \
    MONITOR pMonitor;                                                                                   \
    UINT16 objectFieldCount;                                                                            \
                                                                                                        \
    if(isArrayObject(pObject)) {                                                                        \
        if(isPrimitiveArrayObject(pObject)) {                                                           \
            setObjectMonitorBlack(pMonitor, pObject);                                                   \
            setBlack(getBlockHeader(pObject));                                                          \
            COLLECTOR_LOG(("collector is marking root prim array in block %x",                          \
                getBlockHeader(pObject)));                                                              \
        }                                                                                               \
        else {                                                                                          \
            objectFieldCount = getArrayLength(pObject);                                                 \
            if(objectFieldCount == 0) {                                                                 \
                setObjectMonitorBlack(pMonitor, pObject);                                               \
                setBlack(getBlockHeader(pObject));                                                      \
                COLLECTOR_LOG(("collector is marking root zero length object array in block %x",        \
                    getBlockHeader(pObject)));                                                          \
            }                                                                                           \
            else if(objectFieldCount == 1 && getObjectArrayElement(pObject, 0) == NULL) {               \
                setObjectMonitorBlack(pMonitor, pObject);                                               \
                setBlack(getBlockHeader(pObject));                                                      \
                COLLECTOR_LOG(("collector is marking root object array in block %x",                    \
                    getBlockHeader(pObject)));                                                          \
            }                                                                                           \
            else {                                                                                      \
                setGrey(getBlockHeader(pObject));                                                       \
            }                                                                                           \
        }                                                                                               \
    }                                                                                                   \
    else {                                                                                              \
        objectFieldCount = getObjectFieldCount(pObject);                                                \
        if(objectFieldCount == 0) {                                                                     \
            setObjectMonitorBlack(pMonitor, pObject);                                                   \
            setBlack(getBlockHeader(pObject));                                                          \
            COLLECTOR_LOG(("collector is marking root object in block %x", getBlockHeader(pObject)));   \
        }                                                                                               \
        else if(objectFieldCount == 1 && getObjectFieldObject(getObjectField(pObject, 0)) == NULL) {    \
            setObjectMonitorBlack(pMonitor, pObject);                                                   \
            setBlack(getObjectHeader(pObject, 1));                                                      \
            COLLECTOR_LOG(("collector is marking root object in block %x",                              \
                getObjectHeader(pObject, 1)));                                                          \
        }                                                                                               \
        else {                                                                                          \
            setGrey(getObjectHeader(pObject, objectFieldCount));                                        \
        }                                                                                               \
    }                                                                                                   \
}



static void markClassTable(CLASS_TABLE pClassTable)
{
    UINT16 classTableSize = getClassTableSize(pClassTable);
    UINT16 i;
    UINT16 j;
    UINT16 objectStaticFieldCount;
    CLASS_ENTRY pClassEntry;
    CLASS_INSTANCE pClassInstance;
    COMMON_CLASS_DEF pClassDef;
    JOBJECT_FIELD pField;
    OBJECT_INSTANCE pObject;
    
    /* Search the class table for classes, and their static fields for objects.
     * Monitors in class objects must be marked, because in most cases they will be permanent, but in some
     * cases they will be created by a monitorenter instruction.
     */
    for(i=0; i < classTableSize; i++) {
        pClassEntry = getEntryPtrInTableByIndex(pClassTable, i);
        if(pClassEntry->status != CLASS_STATUS_PREPARED) {
            continue;
        }
        pClassInstance = getClassInstanceFromClassEntry(pClassEntry);
        pClassDef = pClassInstance->pRepresentativeClassDef;
        if(isArrayClass(pClassDef)) { /* array classes have no static fields */
            continue;
        }

        /* class monitors are not collectible so there is no need to blacken them,
         * we just need to blacken any objects in the static fields
         */
        objectStaticFieldCount = getObjectStaticFieldCount(pClassDef);
        for(j=0; j < objectStaticFieldCount; j++) {
            pField = getObjectStaticField(pClassInstance, j);
            pObject = getObjectFieldObject(pField);
            if(pObject == NULL) {
                continue;
            }
            scanObject(pObject);
        }

    }
}




static void markRoots()
{
    THREAD_LIST pJavaThreadList;
    THREAD pThread;
    JAVA_THREAD pJavaThread;
    FRAME pThreadFP;
    JSTACK_FIELD pThreadSP;
    JSTACK_FIELD pThreadLP;
    INSTRUCTION pThreadPC;
    UINT16 currentFieldCount;
    UINT16 i;
    METHOD_DEF pMethod;
    JSTACK_FIELD pEndOfFrame;
    OBJECT_INSTANCE pObject;
    UINT32 *pParameterMap;

    // TODO PROCESS MODEL: do this for all class tables in the process model
    //will need a marker in the class table that indicates the class tables was marked already
    //when marking a previous thread 
    //we must ensure that the class table is marked before any of its corresponding threads or
    //we will have to change the concurrent collection strategy
    markClassTable(getClassTable());

    pJavaThreadList = gcGetActiveThreadList();


    /* note: if gc is concurrent we need not worry about any new threads created during the execution of this 
     * phase, because anything moved onto their stacks will be greyed.
     */

    ITERATE_THREAD_LIST(pJavaThreadList)

        pThread = pJavaThreadList->pThread;
        pJavaThread = (JAVA_THREAD) pThread;
        if(pThread == getCurrentThread()) {
#if LOCAL_FRAME_REGISTERS
            /* we get the register values right off the stack */
            pThreadFP = pFrameRegisterAccessor->fp; 
            pThreadSP = pFrameRegisterAccessor->sp;
            pThreadLP = pFrameRegisterAccessor->lp;
            pThreadPC = pFrameRegisterAccessor->pc;
#else
            pThreadFP = frameRegisters.fp; 
            pThreadSP = frameRegisters.sp;
            pThreadLP = frameRegisters.lp;
            pThreadPC = frameRegisters.pc;
#endif
        }
        else {
            
#if PREEMPTIVE_THREADS && CONCURRENT_GC
            /* we disable the thread so that it does not run
             * while we examine its stack, otherwise the stack would change as 
             * we examine it.
             */
            pThread->savedPriority = getThreadPriority(pThread);
            setPriority(pThread, THREAD_INACTIVE_PRIORITY);

            if(pThread->state == THREAD_FINISHED) {
                /* the thread has died since we obtained our list */
                pJavaThreadList = pJavaThreadList->pNext;
                if(pJavaThreadList == NULL) {
                    break;
                }
                else {
                    continue;
                }
            }
#endif

            /* the thread's registers are accessible in different ways depending on the circumstances */

#if PREEMPTIVE_THREADS && LOCAL_FRAME_REGISTERS
            pThreadFP = pJavaThread->pFrameRegisterAccessor->fp;
            pThreadSP = pJavaThread->pFrameRegisterAccessor->sp;
            pThreadLP = pJavaThread->pFrameRegisterAccessor->lp;
            pThreadPC = pJavaThread->pFrameRegisterAccessor->pc;
#else 
            pThreadFP = pJavaThread->savedJavaRegisters.fp;
            pThreadSP = pJavaThread->savedJavaRegisters.sp;
            pThreadLP = pJavaThread->savedJavaRegisters.lp;
            pThreadPC = pJavaThread->savedJavaRegisters.pc;
#endif

        }


        /* First we mark the thread structure itself.  This memory is collectible because we want it 
         * to disappear once the thread is dead and there are no more java references to it.
         * It is actually an object field of the java.lang.Thread object, it resides in the 
         * memory space where the primitive fields normally reside.
         */
        setBlack(getBlockHeader(getTopJavaThreadStructObject(pThread)));
        COLLECTOR_LOG(("collector is marking thread struct object in block %x", getBlockHeader(getTopJavaThreadStructObject(pThread))));

        /* check the objectToThrow member of the thread,
         * check the wait monitor of the thread,
         * and check the thread object of the thread
         */

        /* mark the system throwable that might not yet be thrown */

        /* By making this first before we mark the thread stack, 
         * we avoid any trouble with concurrent collection since the object's next
         * location can only be somewhere on the thread's stack, which is searched further on in the
         * gc marking process.
         */
        if(pJavaThread->pObjectToThrow != NULL) {
            setBlack(getObjectHeader(pJavaThread->pObjectToThrow, getObjectFieldCount(pJavaThread->pObjectToThrow)));
            COLLECTOR_LOG(("collector is marking system throwable object in block %x", getObjectHeader(pJavaThread->pObjectToThrow, getObjectFieldCount(pJavaThread->pObjectToThrow))));
        }
        
        /* mark the thread object, we know there is at least one non-null object so we mark it grey */
        setGrey(getObjectHeader(pJavaThread->pThreadInstance, getObjectFieldCount(pJavaThread->pThreadInstance)));

        /* mark the monitor being waited on */
        if(pThread->pWaitMonitor != NULL) {
            setBlack(getBlockHeader(pThread->pWaitMonitor));
        }
        pMethod = getFrameMethod(pThreadFP);
        COLLECTOR_LOG(("collector is examining stack of thread"));
        
        do {
            /* check for bottom of stack */
            switch(*pThreadPC) {
                case FIRST_OPCODE:
                    goto markedStack;
            
                case CLINIT_OPCODE:
                case INIT_OPCODE:
                    /* these frames have nothing that we need to examine so we move on */
                    /* these frames are used primarily to construct the frame above, 
                     * see the opcode handlers themselves for more detail
                     */
                    goto nextFrame;
                default:
                    /* fall through */
                    break;
            }

            COLLECTOR_LOG(("collector is examining stack frame"));

            /* scan the method arguments */
            
            pParameterMap = pMethod->pParameterMap;
            currentFieldCount = pMethod->base.parameterVarCount;
            for(i=0; i<currentFieldCount; i++) {
                if(!(pParameterMap[i >> PARAMETER_MAP_INDEX_SHIFT] & 
                    (1 << (i & PARAMETER_MAP_REMAINDER_MASK)))) { /* the argument is a primitive */
                    continue;   
                }
                pObject = getStackFieldObject(pThreadLP + i);
                if(pObject == NULL) {
                    continue;
                }
                scanObject(pObject);
            }

            currentFieldCount += pMethod->localVarCount; /* this is the number of local variables */
            for(; i<currentFieldCount; i++) {
                pObject = getStackFieldObject(pThreadLP + i);

                

#if !USING_GC_STACK_MAPS
                if(pObject == DOUBLE_PRIMITIVE_STACK_INDICATOR) {
                    i++;
                    continue;
                } 
                else
#endif
                /* neither class objects nor their monitors are collectible so we skip those */
                if(pObject == NULL) {
                    continue;
                }

                scanObject(pObject);

            }

            
            
            /* mark the monitor if the frame is synchronized */
            
            if(isSynchronizedMethod(pMethod)) {
                setBlack(getBlockHeader(getMonitorFromFrame(pThreadFP)));
                pEndOfFrame = endOfSynchronizedFrame(pThreadFP);
            }
            else {
                pEndOfFrame = endOfFrame(pThreadFP);
            }
            
            /* scan operand stack */

            currentFieldCount = (pThreadSP - pEndOfFrame) + 1; /* number of entries on operand stack */

            for(i=0; i<currentFieldCount; i++) {
                pObject = getStackFieldObject(pEndOfFrame + i);

                
#if !USING_GC_STACK_MAPS
                if(pObject == DOUBLE_PRIMITIVE_STACK_INDICATOR) {
                    i++;
                    continue;
                } 
                else
#endif
                /* neither class objects nor their monitors are collectible so we skip those */
                if(pObject == NULL) {
                    continue;
                }

                scanObject(pObject);
            }

nextFrame:
            pThreadSP = getPreviousSP(pThreadFP, pThreadLP);
            pThreadPC = getFramePreviousPC(pThreadFP);
            pThreadFP = getFramePreviousFP(pThreadFP);
            pThreadLP = getLocals(pThreadFP);
            pMethod = getFrameMethod(pThreadFP);

        } while(TRUE);

markedStack:

#if PREEMPTIVE_THREADS && CONCURRENT_GC
        if(getCurrentThread() != pThread) {
            setPriority(pThread, pThread->savedPriority);
        }
#endif

    END_ITERATE_THREAD_LIST(pJavaThreadList)
    
    
}


/* we scan through the heap to find all greyed object blocks.
 *
 * For each greyed block, we traverse through the tree of all object fields to blacken any object that
 * is reachable from the top-level greyed object.
 */
static void markChildren()
{
    OBJECT_INSTANCE pObject;
    BLOCK_HEADER pBlock = pHeap;
    BLOCK_HEADER pCurrentBlock;
    BLOCK_HEADER pFieldBlock;
    UINT16 fieldCounter;
    UINT16 fieldCount;
    BOOLEAN isObjectArray;
    OBJECT_INSTANCE pFieldObject;
    JOBJECT_FIELD pField;
    MONITOR pMonitor;
                    
    /* we scan the heap looking for greyed blocks */

checkNextBlock:
    switch(getFlags(pBlock)) {
        case GREYED_SIMPLE_OBJECT_MASK:
            /* 
             * We might arrive here if during concurrent gc an object or array with no object fields
             * was greyed. 
             */
            pObject = getBlockData(pBlock);
            setObjectMonitorBlack(pMonitor, pObject);
            setBlack(pBlock);
            COLLECTOR_LOG(("collector is marking simple object from heap in block %x", pBlock));
            pBlock = getFollowingBlock(pBlock);
            if(isNotPastHeapEnd(pBlock)) {
                goto checkNextBlock;
            }
            break;

        case GREYED_ARRAY_FIELDS_MASK: /* a greyed object array */
            pObject = getBlockData(pBlock);
            isObjectArray = TRUE;
            COLLECTOR_LOG(("collector is marking object array from heap in block %x with %d array elements", pBlock, getContainedObjectCount(pBlock)));
            goto markChildren;

        case GREYED_OBJECT_FIELDS_MASK: /* a greyed object */
            pObject = (OBJECT_INSTANCE) (((JOBJECT_FIELD) getBlockData(pBlock)) + getContainedObjectCount(pBlock));
            isObjectArray = FALSE;
            COLLECTOR_LOG(("collector is marking object from heap in block %x with %d object fields", pBlock, getContainedObjectCount(pBlock)));
                        
            
markChildren:
            /* we must blacken all object fields */
    
            setBlack(pBlock);
            setObjectMonitorBlack(pMonitor, pObject);
            pBlock->back.pContainingObjectBlock = NULL;
            fieldCounter = 0;
            fieldCount = getContainedObjectCount(pBlock);
            pCurrentBlock = pBlock;
            goto markFirstField; /* the flags indicate there is at least one object field */

            
markField:


            /* check if we have checked all object fields of the current object */
            if(fieldCounter < fieldCount) {

markFirstField:            
                if(isObjectArray) {
                    pField = getObjectArrayField(pObject, fieldCounter);
                    pFieldObject = getObjectFieldObject(pField);
                }
                else {
                    pField = getObjectField(pObject, fieldCounter);
                    pFieldObject = getObjectFieldObject(pField);
                }
                fieldCounter++;

                if(pFieldObject != NULL) { /* we will blacken this non-null field if it is not blackened already */
                    UINT16 containedObjectCount;
                    BOOLEAN isFieldObjectArray = (isArrayObject(pFieldObject) != 0);
                    if(isFieldObjectArray) {
                        pFieldBlock = getBlockHeader(pFieldObject);
                        if(isBlack(pFieldBlock)) {
                            goto markField;
                        }
                        if(isPrimitiveArrayObject(pFieldObject)) {
                            containedObjectCount = 0;
                            COLLECTOR_LOG(("collector is marking referenced prim array in block %x", pFieldBlock));
                        }
                        else {
                            containedObjectCount = getArrayLength(pFieldObject);
                            /* this quick check saves us some time later */
                            if(containedObjectCount == 1 && getObjectArrayElement(pFieldObject, 0) == NULL) {
                                containedObjectCount = 0;
                            }
                            COLLECTOR_LOG(("collector is marking referenced object array in block %x with %d array elements", pFieldBlock, containedObjectCount));
                        }
                    }
                    else {
                        containedObjectCount = getObjectFieldCount(pFieldObject);
                        pFieldBlock = getObjectHeader(pFieldObject, containedObjectCount);
                        if(isBlack(pFieldBlock)) {
                            goto markField;
                        }
                        COLLECTOR_LOG(("collector is marking object in block %x with %d object fields", pFieldBlock, containedObjectCount));
                        if(containedObjectCount == 1 && getObjectFieldObject(getObjectField(pFieldObject, 0)) == NULL) {
                            containedObjectCount = 0;
                        }
                    }
                    setBlack(pFieldBlock);
                    setObjectMonitorBlack(pMonitor, pFieldObject);        
                    if(containedObjectCount) { /* the field object contains other objects */

                        /* We will jump down in the tree into the field object to blacken its respective fields.
                         * When we are done we will jump back and continue looking at the fields of the current object.
                         */
                        pFieldBlock->back.pContainingObjectBlock = pCurrentBlock;
                        pCurrentBlock->forward.indices.currentIndex = fieldCounter;
                        pCurrentBlock = pFieldBlock;
                        pObject = pFieldObject;
                        isObjectArray = isFieldObjectArray;
                        fieldCounter = 0;
                        fieldCount = containedObjectCount;
                    }              
                }
                goto markField;
            }
            else if(pCurrentBlock->back.pContainingObjectBlock != NULL) {
                /* We have blackened all fields of the current object.
                 * We will jump back into the containing object.
                 */
                pCurrentBlock = pCurrentBlock->back.pContainingObjectBlock;
                fieldCounter = pCurrentBlock->forward.indices.currentIndex;
                fieldCount = getContainedObjectCount(pCurrentBlock);
                switch(getFlags(pCurrentBlock)) {
                    case BLACK_OBJECT_FIELDS_MASK:
                    case BLACK_GREY_OBJECT_FIELDS_MASK:
                        isObjectArray = FALSE;
                        pObject = (OBJECT_INSTANCE) (((JOBJECT_FIELD) getBlockData(pCurrentBlock)) + getContainedObjectCount(pCurrentBlock));
                        break;
                    case BLACK_ARRAY_FIELDS_MASK:
                    case BLACK_GREY_ARRAY_FIELDS_MASK:
                        isObjectArray = TRUE;
                        pObject = getBlockData(pCurrentBlock);
                        break;
                    default:
                        /* we should never arrive here */
                        break;
                }
                goto markField;
            }

            /* fall through */

        default:
            /* at this point, exactly one of the following are true:
             * - the block is free
             * - the block is allocated but not collectible
             * - the block is allocated and is black
             *
             * If the block was allocated and grey, it is now black as well.
             */
            pBlock = getFollowingBlock(pBlock);
            if(isNotPastHeapEnd(pBlock)) {
                goto checkNextBlock;
            }
    }            
}

RETURN_CODE collectibleMemoryCallocMonitor(MONITOR *ppMonitor)
{
    RETURN_CODE ret;

    enterMonitorAllocationMonitor();
    if(*ppMonitor) {
        exitMonitorAllocationMonitor();
        return SUCCESS;
    }

    ret = collectibleMemoryAlloc(sizeof(monitorStruct), REGULAR_COLLECTIBLE_BITS, 0, 0, ppMonitor);
    if(ret == SC_SUCCESS) {
        exitMonitorAllocationMonitor();
        monitorInit(*ppMonitor);
        return SUCCESS;
    }

    exitMonitorAllocationMonitor();
    return ERROR_CODE_OUT_OF_MEMORY;
}

RETURN_CODE collectibleMemoryCallocArray(UINT32 byteSize, UINT16 numContainedObjects, ARRAY_INSTANCE *ppInstance)
{
    RETURN_CODE ret;

    if(numContainedObjects) {
        ret = collectibleMemoryAlloc(byteSize, OBJECT_ARRAY_WITH_FIELDS_BIT, numContainedObjects, 0, ppInstance);
    }
    else {
        ret = collectibleMemoryAlloc(byteSize, REGULAR_COLLECTIBLE_BITS, 0, 0, ppInstance);
    }
    if(ret == SUCCESS) {
        memorySet(*ppInstance, 0, byteSize);
    }
    return ret;
}

RETURN_CODE collectibleMemoryCallocObject(UINT32 byteSize, UINT16 objectFieldCount, OBJECT_INSTANCE *ppInstance)
{
    RETURN_CODE ret;
    if(objectFieldCount) {
        ret = collectibleMemoryAlloc(byteSize, OBJECT_WITH_OBJECT_FIELDS_BIT, objectFieldCount, objectFieldCount * getObjectFieldSize(), ppInstance);
        if(ret == SUCCESS) {
            memorySet(((JOBJECT_FIELD) *ppInstance) - objectFieldCount, 0, byteSize);
        }
    }
    else {
        ret = collectibleMemoryAlloc(byteSize, REGULAR_COLLECTIBLE_BITS, 0, 0, ppInstance);
        if(ret == SUCCESS) {
            memorySet(*ppInstance, 0, byteSize);
        }
    }
    return ret;

}

void javaMarkFunction() 
{
    whiten();
    markRoots();
    markChildren();

#if PREEMPTIVE_THREADS && CONCURRENT_GC
    /* During the markChildren phase, a block could have been transferred to a root, or to a child which had been already
     * scanned.  Such blocks will be greyed (all such operations automatically
     * grey their respective objects if gc is concurrent) and must be scanned
     * for children as well.  There should not be many such blocks and we
     * disable all other java threads during this phase so that we need not repeat this phase ad infinitum.
     */
    disableNonCurrentJavaThreads();
    markChildren();
    reenableNonCurrentJavaThreads();
#endif

}

#if LOGGING


void logHeap()
{
    FREE_LIST_BLOCK_HEADER pFreeBlock = pFreeList;
    BLOCK_HEADER pBlock = asBlockPtr(pHeap);
    UINT8 flags;
    char collectibleObject[] = "collectible object";
    char collectibleObjectArray[] = "collectible object array";
    char nonCollectible[] = "non-collectible";
    char collectible[] = "collectible";
    char free[] = "free";
    char unknown[] = "unknown"; 
    char *str;
    UINT32 freeListFreeCount = 0;
    UINT32 heapFreeCount = 0;
    UINT32 heapCollectibleCount = 0;
    UINT32 heapNonCollectibleCount = 0;
    UINT32 heapCollectibleBytes = 0;
    UINT32 heapNonCollectibleBytes = 0;
    UINT32 heapFreeBytes = 0;

    LOG_LINE(("\nfree list"));

    while(pFreeBlock != NULL) {
        flags = getFlags(pFreeBlock);
        freeListFreeCount++;
        LOG_LINE(("free block %x flags %x length %d pcontig %x pfree %x nfree %x", 
            pFreeBlock, flags, getSize(pFreeBlock), pFreeBlock->header.back.pPreviousContiguous, 
            pFreeBlock->pPreviousFree, pFreeBlock->header.forward.pNextFree));
        pFreeBlock = pFreeBlock->header.forward.pNextFree;
    }
    
    while(isNotPastHeapEnd(pBlock)) {
        flags = getAllocFlags(pBlock);
        switch(flags) {
            case NON_COLLECTIBLE_BIT:
                heapNonCollectibleCount++;
                heapNonCollectibleBytes += getSize(pBlock);
                str = nonCollectible;
                break;
            case OBJECT_WITH_OBJECT_FIELDS_BIT:
                heapCollectibleCount++;
                heapCollectibleBytes += getSize(pBlock);
                str = collectibleObject;
                break;
            case OBJECT_ARRAY_WITH_FIELDS_BIT:
                heapCollectibleCount++;
                heapCollectibleBytes += getSize(pBlock);
                str = collectibleObjectArray;
                break;
            case REGULAR_COLLECTIBLE_BITS:
                heapCollectibleCount++;
                heapCollectibleBytes += getSize(pBlock);
                str = collectible;
                break;
            case 0:
                str = free;
                heapFreeCount++;
                heapFreeBytes += getSize(pBlock);
                break;
            default:
                str = unknown;
                break;
        }
        LOG_LINE(("block %x length %d end %x, type %s", pBlock, getSize(pBlock), ((PBYTE) pBlock) + getSize(pBlock), str));
        if(getSize(pBlock) == 0) {
            LOG_LINE(("Block of minimum size resulting from allocation request of 0 bytes"));
        }
        pBlock = getFollowingBlock(pBlock);
    }
    if(freeListFreeCount != heapFreeCount) {
        LOG_LINE(("uneneven free counts: heap - %d free list - %d\n", heapFreeCount, freeListFreeCount));
    }
    LOG_LINE(("%d free blocks of total size %d", heapFreeCount, heapFreeBytes));
    LOG_LINE(("%d collectible blocks of total size %d", heapCollectibleCount, heapCollectibleBytes));
    LOG_LINE(("%d non-collectible blocks of total size %d\n", heapNonCollectibleCount, heapNonCollectibleBytes));
}


#endif

