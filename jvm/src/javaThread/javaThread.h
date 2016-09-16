#ifndef JAVATHREAD_H
#define JAVATHREAD_H


#include "thread/thread.h"
#include "interpret/javaGlobals.h"

typedef struct stackSegmentStruct stackSegmentStruct, *STACK_SEGMENT;

/* the common elements of all stack frames */

/* Note: the METHOD_DEF must come first in the structure below, see the comment situated above the 
 * definition of jstackField for an explanation.
 */
struct frameStruct {
    METHOD_DEF pMethod;
    FRAME previousFP;
    INSTRUCTION previousPC;
#if GROW_STACK
    JSTACK_FIELD previousSP;  /* will be lp - 1 if stack contiguous (no growth) */
    JSTACK_FIELD pStackLimit; /* stored in the thread structure if stack contiguous (no growth) */
#endif
};

#define getFrameMethod(fp) ((fp)->pMethod)
#define getFramePreviousFP(fp) ((fp)->previousFP)
#define getFramePreviousPC(fp) ((fp)->previousPC)
#define setFrameMethod(fp, pMeth) ((fp)->pMethod = (pMeth))
#define setFramePreviousFP(fp, prevFP) ((fp)->previousFP = (prevFP))
#define setFramePreviousPC(fp, prevPC) ((fp)->previousPC = (prevPC))

#if GROW_STACK
#define getFramePreviousSP(fp) ((fp)->previousSP)
#define getFrameStackLimit(fp) ((fp)->pStackLimit)
#define setFramePreviousSP(fp, prevSP) ((fp)->previousSP = (prevSP))
#define setFrameStackLimit(fp, sl) ((fp)->pStackLimit = (sl))
#define getPreviousSP(fp, lp) getFramePreviousSP(fp)
#else
#define getPreviousSP(fp, lp) ((lp) - 1)
#endif

typedef struct synchronizedFrameStruct {
    frameStruct frame;
    MONITOR pMonitor;
} synchronizedFrameStruct, *SYNCHRONIZED_FRAME;


#define endOfFrame(fp) ((void *) (((FRAME) (fp)) + 1))
#define endOfSynchronizedFrame(fp) ((void *) (((SYNCHRONIZED_FRAME) (fp)) + 1))
#define getLocals(fp) (((JSTACK_FIELD) (fp)) - getFrameMethod(fp)->varCount)


#define getMonitorFromFrame(fp) (((SYNCHRONIZED_FRAME) (fp))->pMonitor)
#define setMonitorInFrame(fp, pAssignMonitor) (((SYNCHRONIZED_FRAME) (fp))->pMonitor = pAssignMonitor)



/* total amount of stack space required to execute a method */

#define getInvokeSize(parametersAndLocals, maxStack, isSynchronized) \
    (parametersAndLocals + maxStack + (isSynchronized ? jstackFieldSizeOf(synchronizedFrameStruct) : jstackFieldSizeOf(frameStruct)))



/* Java Threads */


/* these correspond with the same values defined in the java.lang.Thread class */
typedef enum javaPriorityEnum {
    JAVA_PRIORITY_LOWEST = 1,
    JAVA_PRIORITY_LOW = 2,
    JAVA_PRIORITY_MID_LOW = 3,
    JAVA_PRIORITY_NORMAL = 4,
    JAVA_PRIORITY_MID_HIGH = 5,
    JAVA_PRIORITY_HIGH = 6,
    JAVA_PRIORITY_HIGHEST = 7
} javaPriorityEnum;


/* 
 * The structure below defines additional parameters for a thread running within the java virtual machine.
 *
 * Note: a java.lang.Thread object has this javaThreadStruct stored as one of its private fields, located 
 * in the primitive field data area of a larger-than-usual java.lang.Object.
 *
 * The garbage collector will eventually collect the structure along with its corresponding java.lang.Thread object.
 *
 */
struct javaThreadStruct {
    threadStruct header;

    /* the beginning of the Java stack */
    STACK_SEGMENT pStackBase;

    /* these priorities do not map directly to kernel priorities,
     * they map to kernel time shares (see USE_TIME_SHARES parameter description)
     */
    javaPriorityEnum javaPriority;

    
    /* instance of java.lang.Thread */
    OBJECT_INSTANCE pThreadInstance;     

    /* a repository safe from garbage collection for a VM created throwable instance being thrown */
    OBJECT_INSTANCE pObjectToThrow;


#if !GROW_STACK
    JSTACK_FIELD pStackLimit;
#endif

    /* we need to save the registers here for the following scenarios: 
     * - for thread switching, the new running thread gets its registers from here, 
     * the switched out thread saves them here - whether this occurs depends on the VM configuration
     * - registers are initialized here when creating a new thread or when trapping into the execution loop from hardware
    */
    frameRegisterStruct savedJavaRegisters;

#if LOCAL_FRAME_REGISTERS && PREEMPTIVE_THREADS
    /* Since the frame registers are local, they must be made available to components such as the
     * garbage collector.  Otherwise, they can be accessed through the global frame registers.
     */
    FRAME_REGISTERS pFrameRegisterAccessor;

#endif

#if !PREEMPTIVE_THREADS
    /* the following flag indicates we will be executing the same native method instruction a second time */
    BOOLEAN blockedOnNative;
#endif

    /* all active java threads are found in the active list at all times */
    threadListStruct activeJavaListStruct;

    /* used so that the garbage collector can get a snapshot of the active threads */
    threadListStruct gcListStruct;
};




void setJavaPriority(JAVA_THREAD pThread, javaPriorityEnum priority);
javaPriorityEnum getJavaPriority(JAVA_THREAD pThread);


#define initializeJavaThreadStruct(pThread, pSwitchOut, pSwitchIn, pCleanUp, kernelPriority) {              \
    initializeThreadStruct((THREAD) pThread, (void (*)(THREAD)) pSwitchOut,                                 \
        (void (*)(THREAD)) pSwitchIn, pCleanUp, kernelPriority);                                            \
    (pThread)->javaPriority = JAVA_PRIORITY_NORMAL;                                                         \
    (pThread)->activeJavaListStruct.pThread = (THREAD) (pThread);                                           \
    (pThread)->gcListStruct.pThread = (THREAD) (pThread);                                                   \
}

/* the thread structure resides in memory as part of a typical garbage collectible java.lang.Object, 
 * where the primitive fields normally reside.  This object resides in an object field of the java.lang.Thread
 * object,  This allows the structure to persists as long as the java.lang.Thread object persists,
 * even after the thread is dead, so that any java.lang.Thread native functions calls behave as usual.
 */

/* The thread structure starts at the primitive fields of a regular object.  
 * So the object instance is found by subtracting this offset.
 */
#define getJavaThreadStructObject(pThread) ((OBJECT_INSTANCE) (((PBYTE) (pThread)) - structOffsetOf(objectInstanceStruct, primitiveFields)))

/* We must also account object fields (of which there is none at this time) to get the top of the object. */
#define getTopJavaThreadStructObject(pThread) getObjectField(getJavaThreadStructObject(pThread), getObjectInstanceFieldCount(pJavaLangObjectDef) - 1)

/* determines if the thread struct field member has been populated yet */
#define containsThreadStruct(pObject) (extractObjectFieldObject((pObject)->javaThreadStructObject) != NULL)

/* The thread structure starts where the primitive fields start in the object.  The rest of the object must not
 * be overwritten so that the garbage collector and anyone else sees the object just like any other object.
 */
#define getThreadStructFromThreadObject(pObject) ((JAVA_THREAD) (extractObjectFieldObject((pObject)->javaThreadStructObject)->primitiveFields))

/* first convert the object instance to a java.lang.Thread instance by accounting for the object fields */
#define getThreadStructFromObject(pObject) getThreadStructFromThreadObject(getThreadInstanceFromObjectInstance(pObject))



#define getCurrentJavaThread() ((JAVA_THREAD) getCurrentThread())

#define currentJavaThreadIsInterrupted() (getCurrentThread()->interruptedFlag)
#define clearCurrentJavaThreadInterruptedFlag() (getCurrentThread()->interruptedFlag = FALSE)
#define monitorNotOwnedByCurrentJavaThread(pMonitor) ((pMonitor)->pOwner != getCurrentThread())
#define monitorOwnedByCurrentJavaThread(pMonitor) ((pMonitor)->pOwner == getCurrentThread())


UINT32 getActiveJavaThreadCount();
void scheduleNewJavaThread(JAVA_THREAD pNewThread, MONITOR pMonitor);
void removeJavaThread();
void disableNonCurrentJavaThreads();
void reenableNonCurrentJavaThreads();
THREAD_LIST gcGetActiveThreadList();






/* saved frame register access */


#define setJavaThreadLP(pThread, savedLP)                           ((pThread)->savedJavaRegisters.lp = (savedLP))
#define setJavaThreadFP(pThread, savedFP)                           ((pThread)->savedJavaRegisters.fp = (savedFP))
#define setJavaThreadSP(pThread, savedSP)                           ((pThread)->savedJavaRegisters.sp = (savedSP))
#define setJavaThreadPC(pThread, savedPC)                           ((pThread)->savedJavaRegisters.pc = (savedPC))

#define getJavaThreadLP(pThread)                                    ((pThread)->savedJavaRegisters.lp)
#define getJavaThreadFP(pThread)                                    ((pThread)->savedJavaRegisters.fp)
#define getJavaThreadSP(pThread)                                    ((pThread)->savedJavaRegisters.sp)
#define getJavaThreadPC(pThread)                                    (((JAVA_THREAD) pThread)->savedJavaRegisters.pc)


#if USE_CLASS_REGISTERS

#define setJavaThreadCurrentMethodDef(pThread, pMethodDef)          ((pThread)->savedJavaRegisters.pCurrentMethodDef = (pMethodDef))
#define setJavaThreadCurrentClassDef(pThread, pClassDef)            ((pThread)->savedJavaRegisters.pCurrentClassDef = (pClassDef))
#define setJavaThreadCurrentConstantPool(pThread, pConstantPool)    ((pThread)->savedJavaRegisters.pCurrentConstantPool = (pConstantPool))

#define getJavaThreadCurrentMethodDef(pThread)                      ((pThread)->savedJavaRegisters.pCurrentMethodDef)
#define getJavaThreadCurrentClassDef(pThread)                       ((pThread)->savedJavaRegisters.pCurrentClassDef)
#define getJavaThreadCurrentConstantPool(pThread)                   ((pThread)->savedJavaRegisters.pCurrentConstantPool)

#else

#define setJavaThreadCurrentMethodDef(pThread, pMethodDef)
#define setJavaThreadCurrentClassDef(pThread, pClassDef)
#define setJavaThreadCurrentConstantPool(pThread, pConstantPool)

#define getJavaThreadCurrentMethodDef(pThread)                      (getJavaThreadFP(pThread)->pMethod)
#define getJavaThreadCurrentClassDef(pThread)                       (getJavaThreadCurrentMethodDef(pThread)->base.pOwningClass)
#define getJavaThreadCurrentConstantPool(pThread)                   (getJavaThreadCurrentClassDef(pThread)->loadedHeader.pConstantPool)

#endif

#define setJavaThreadRegisters(pThread, fp, pMethod) {                                                  \
    setJavaThreadSP(pThread, (((JSTACK_FIELD) endOfFrame(fp)) - 1));                                    \
    setJavaThreadPC(pThread, pMethod->pCode);                                                           \
    setJavaThreadFP(pThread, fp);                                                                       \
    setJavaThreadLP(pThread, getLocals(fp));                                                            \
    setJavaThreadCurrentMethodDef(pThread, pMethod);                                                    \
    setJavaThreadCurrentClassDef(pThread, pMethod->base.pOwningClass);                                  \
    setJavaThreadCurrentConstantPool(pThread, pMethod->base.pOwningClass->loadedHeader.pConstantPool);  \
}


#define pushJavaThreadOperandStackObject(pThread, x) setStackFieldObject(++getJavaThreadSP(pThread), x)
#define pushJavaThreadOperandStackInt(pThread, x) setStackFieldInt(++getJavaThreadSP(pThread), x)







#endif