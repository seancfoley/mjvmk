#ifndef EXECUTE_H
#define EXECUTE_H

#include "operate/operate.h"
#include "interpret/returnCodes.h"
#include "thread/thread.h"
#include "memory/heap.h"
#include "frame/stackSegment.h"


#if USING_GC_STACK_MAPS

#define clearOpStack(pStackField, maxEntries)

#else

#define clearOpStack(pStackField, maxEntries) {                     \
    if(maxEntries) {                                                \
        JSTACK_FIELD pOpField = (JSTACK_FIELD) (pStackField);       \
        JSTACK_FIELD pOpFieldEnd = pOpField + (maxEntries);         \
        do {                                                        \
            setStackFieldObject(pOpField, NULL);                    \
            pOpField++;                                             \
        } while(pOpField < pOpFieldEnd);                            \
    }                                                               \
}

#endif


#if GROW_STACK || CHECK_STACK

#if USING_GC_STACK_MAPS

#define getFrameStruct(pNewFrameVar, newLP, parameterCount, localCount, totalCount)     \
    (pNewFrameVar = (FRAME) (newLPVar + totalCount))

#else

/* we must set all local variable object pointes to NULL */
#define getFrameStruct(pNewFrameVar, newLP, parameterCount, localCount, totalCount) {   \
    if(localCount) {                                                                    \
        JSTACK_FIELD pLocalField;                                                       \
        pLocalField = newLP + parameterCount;                                           \
        pNewFrameVar = (FRAME) (pLocalField + localCount);                              \
        do {                                                                            \
            setStackFieldObject(pLocalField, NULL);                                     \
            pLocalField++;                                                              \
        }  while(pLocalField < (JSTACK_FIELD) pNewFrameVar);                            \
    }                                                                                   \
    else {                                                                              \
        pNewFrameVar = (FRAME) (newLP + parameterCount);                                \
    }                                                                                   \
}

#endif 

#else /* GROW_STACK || CHECK_STACK */

#if USING_GC_STACK_MAPS

#define getFrameStruct(pNewFrameVar, sp, localCount)                                    \
    (pNewFrameVar = (FRAME) (sp + localCount + 1))

#else

#define getFrameStruct(pNewFrameVar, sp, localCount) {                                  \
    if(localCount) {                                                                    \
        JSTACK_FIELD pLocalField = sp + 1;                                              \
        pNewFrameVar = (FRAME) (pLocalField + localCount);                              \
        do {                                                                            \
            setStackFieldObject(pLocalField, NULL);                                     \
            pLocalField++;                                                              \
        } while(pLocalField < (JSTACK_FIELD) pNewFrameVar);                             \
    }                                                                                   \
    else {                                                                              \
        pNewFrameVar = (FRAME) (sp + 1);                                                \
    }                                                                                   \
}

#endif /* USING_GC_STACK_MAPS */

#endif /* GROW_STACK || CHECK_STACK */







#define popFrame(fp, lp, retLength) {                                                           \
    setSP(getPreviousSP(fp, lp) + retLength);                                                   \
    setPC(getFramePreviousPC(fp));                                                              \
    setFP(getFramePreviousFP(fp));                                                              \
    setLP(getLocals(getFP()));                                                                  \
    setCurrentMethodDef(getFrameMethod(getFP()));                                               \
    setCurrentClassDef(getCurrentMethodDef()->base.pOwningClass);                               \
    setCurrentConstantPool(getCurrentClassDef()->loadedHeader.pConstantPool);                   \
}

#define initializeFrame(newFP, preFP, returnPC, pMethodDef) {                                   \
    setFrameMethod(newFP, pMethodDef);                                                          \
    setFramePreviousFP(newFP, preFP);                                                           \
    setFramePreviousPC(newFP, returnPC);                                                        \
}

#define initializeLargeFrame(newFP, preFP, returnPC, pMethodDef, preSP, pSL) {                  \
    initializeFrame(newFP, preFP, returnPC, pMethodDef);                                        \
    setFramePreviousSP(newFP, preSP);                                                           \
    setFrameStackLimit(newFP, pSL);                                                             \
}


#if GROW_STACK

#define pushFrame(pNewFrameVar, pMethodDef, sp, fp, pc, sl, failureRoutine) {                           \
    JSTACK_FIELD newStackLimit = sl;                                                                    \
    UINT16 parameterVarCount = pMethodDef->base.parameterVarCount;                                      \
    JSTACK_FIELD newLP = sp - parameterVarCount + 1;                                                    \
                                                                                                        \
    if(stackAllocationIsRequired(newLP, newStackLimit, pMethodDef->invokeCount) &&                      \
        !(newLP = startNewStackSegment(pMethodDef, sp + 1, sl, &newStackLimit))) {                      \
        failureRoutine(ERROR_CODE_STACK_OVERFLOW);                                                      \
    }                                                                                                   \
    else {                                                                                              \
        getFrameStruct(pNewFrameVar, newLP, pMethodDef->base.parameterVarCount,                         \
            pMethodDef->localVarCount, pMethodDef->varCount);                                           \
        clearOpStack(pNewFrameVar + 1, pMethodDef->maxStackWithMonitor);                                \
        initializeLargeFrame(pNewFrameVar, fp, pc, pMethodDef, sp - parameterVarCount, newStackLimit);  \
    }                                                                                                   \
}


#else

#if CHECK_STACK

#define pushFrame(pNewFrameVar, pMethodDef, sp, fp, pc, sl, failureRoutine) {                       \
    JSTACK_FIELD newLP = sp - pMethodDef->base.parameterVarCount + 1;                               \
                                                                                                    \
    if(stackAllocationIsRequired(newLP, sl, pMethodDef->invokeCount)) {                             \
        failureRoutine(ERROR_CODE_STACK_OVERFLOW);                                                  \
    }                                                                                               \
    else {                                                                                          \
        getFrameStruct(pNewFrameVar, newLP, pMethodDef->base.parameterVarCount,                     \
            pMethodDef->localVarCount, pMethodDef->varCount);                                       \
        clearOpStack(pNewFrameVar + 1, pMethodDef->maxStackWithMonitor);                            \
        initializeFrame(pNewFrameVar, fp, pc, pMethodDef);                                          \
    }                                                                                               \
}

#else

#define pushFrame(pNewFrameVar, pMethodDef, sp, fp, pc, sl, failureRoutine) {                       \
    getFrameStruct(pNewFrameVar, sp, pMethodDef->localVarCount);                                    \
    clearOpStack(pNewFrameVar + 1, pMethodDef->maxStackWithMonitor);                                \
    initializeFrame(pNewFrameVar, fp, pc, pMethodDef);                                              \
}

#endif

#endif


/* the possible stages in a class initialization, in order */
typedef enum clinitStageEnum {
    CLINIT_INITIAL_STAGE,
    CLINIT_ENTERED_MONITOR,
    CLINIT_COMPLETED_SUPER_CLINIT,
    CLINIT_COMPLETED_CLINIT,
    CLINIT_ENTERED_MONITOR_UPON_COMPLETION,
    CLINIT_FAILED,
    CLINIT_ENTERED_MONITOR_UPON_FAILURE
} clinitStageEnum, CLINIT_STAGE_CODE;


typedef struct clinitDataStruct {
    /* the thrown object entry must be first here 
     * because that is where it will appear on the stack when it is thrown
     */
    jstackField thrownThrowableObject;      
    jstackField classInstanceObject;
    jstackField clinitStageCode;
} clinitDataStruct, *CLINIT_DATA;



void executeCode();
void initializeExecution();
void prepareSwitchJavaThreadOut(JAVA_THREAD pThread);
void prepareSwitchJavaThreadIn(JAVA_THREAD pThread);
RETURN_CODE setupJavaThread(OBJECT_INSTANCE pThreadObject, JAVA_THREAD pThread, METHOD_DEF pInitialMethod, UINT16 initialArgCount, JSTACK_FIELD initialArgs, UINT32 initialStackSize);
RETURN_CODE startNewThread(OBJECT_INSTANCE pThreadObject, UINT32 initialStackSize);
RETURN_CODE startThread(OBJECT_INSTANCE pThreadObject, JAVA_THREAD pThread, METHOD_DEF pInitialMethod, 
                        MONITOR pMonitor, UINT16 initialArgCount, JSTACK_FIELD initialArgs, 
                        UINT32 initialStackSize);



/* Instantiate an object, but add space for the thread structure.
 * The thread structure will occupy the space where the primitive fields normally reside.
 * The object will appear as a field in the java.lang.Thread object.  
 * See the javaThreadStruct comments for more details.  
 */
#define createThreadStruct(pThreadObject)                                                                       \
    instantiateObject(pJavaLangObject, extractObjectFieldObjectPtr((pThreadObject)->javaThreadStructObject),    \
    (UINT16) (getInstantiateSize(pJavaLangObjectDef) + sizeof(javaThreadStruct)), FALSE)


extern METHOD_DEF pInternalInitMethod;
extern METHOD_DEF pInternalClinitMethod;
extern METHOD_DEF pIdleMethod;

#endif