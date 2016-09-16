
#include "thread/schedule.h"
#include "thread/thread.h"
#include "thread/timer.h"
#include "interpret/instructions.h"
#include "javaThread/javaThread.h"
#include "memory/memoryAlloc.h"
#include "memory/garbageCollect.h"
#include "object/allocate.h"
#include "object/instantiate.h"
#include "string/stringKeys.h"
#include "log/logItem.h"
#include "class/table.h"
#include "class/nativeTable.h"
#include "class/typeCheck.h"
#include "resolution/resolve/resolve.h"
#include "resolution/resolve/lookup.h"
#include "resolution/constantPool.h"
#include "handlers/handlers.h"
#include "javaRegisterSwitch.h"
#include "execute.h"




/* special methods */




/* the following handler list will catch everything and return execution to the first bytecode in a method */
static throwableHandlerStruct catchAllExceptionHandler;
static throwableHandlerListStruct catchAllHandlerList;


/* the first method is used to construct the first stack frame for a new thread */
static byte firstBytecode = FIRST_OPCODE;
static methodDefStruct firstMethod;

/* the internal method is used to handle inits required by the Class.newInstance native method */
#define INTERNAL_INIT_STACK_ENTRIES 1 /* need operand stack space for the object instance */
static byte internalInitBytecode = INIT_OPCODE;
static methodDefStruct internalInitMethod;
METHOD_DEF pInternalInitMethod;

/* the clinit method is used to handle clinits */
#define INTERNAL_CLINIT_STACK_ENTRIES jstackFieldSizeOf(clinitDataStruct)
static byte internalClinitBytecode = CLINIT_OPCODE;
static methodDefStruct internalClinitMethod;
METHOD_DEF pInternalClinitMethod;

/* the idle method is the only method that the idle thread will execute */
static byte idleBytecode = IDLE_OPCODE;
static methodDefStruct idleMethod;
METHOD_DEF pIdleMethod;


/* these methods are never executed, they are used in specially constructed java stack frames that
 * indicate special conditions during the execution of a particular thread.
 */
static void constructCatchAllMethod(METHOD_DEF pMethod, THROWABLE_HANDLER_LIST pHandlerList, INSTRUCTION pCode, UINT16 maxStack)
{
    pMethod->base.parameterVarCount = 0; /* ensures sp is set correctly when frame is popped */
    pMethod->base.pOwningClass = pJavaLangThreadDef;
    pMethod->pCode = pCode;
    pMethod->byteCodeCount = 1;
    pMethod->throwableHandlers = pHandlerList;
    pMethod->invokeCount = getInvokeSize(0, maxStack, FALSE);
    pMethod->localVarCount = 0;
    return;
}

static void constructCatchAllHandlerList(THROWABLE_HANDLER_LIST pHandlerList, THROWABLE_HANDLER pHandler)
{
    pHandlerList->length = 1;
    pHandlerList->ppHandlers[0] = pHandler;
    pHandler->catchKey.namePackageKey = 0; /* catches everything */
    pHandler->startPCIndex = 0; /* handler covers the entire method (which is only one byte long) */
    pHandler->endPCIndex = 1;
    pHandler->handlerPCIndex = 0; /* will set the pc to point to the sole byte code */
    return;
}

void initializeExecution()
{
    pInternalInitMethod = &internalInitMethod;
    pInternalClinitMethod = &internalClinitMethod;
    pIdleMethod = &idleMethod;
    constructCatchAllHandlerList(&catchAllHandlerList, &catchAllExceptionHandler);
    constructCatchAllMethod(&firstMethod, &catchAllHandlerList, &firstBytecode, 0);
    constructCatchAllMethod(pInternalInitMethod, &catchAllHandlerList, &internalInitBytecode, INTERNAL_INIT_STACK_ENTRIES);
    constructCatchAllMethod(&idleMethod, &catchAllHandlerList, &idleBytecode, 0);
    constructCatchAllMethod(pInternalClinitMethod, &catchAllHandlerList, &internalClinitBytecode, INTERNAL_CLINIT_STACK_ENTRIES);    
}




#if !PREEMPTIVE_THREADS

UINT32 timeCounter = 0;
BOOLEAN signalledJavaRegisterSwitch = FALSE;
BOOLEAN requestedJavaRegisterSwitch = FALSE;

#endif




/* This function is the entry point for all new threads */

/* This function can also serve as a trap if certain opcodes are handled by hardware,
 * however it might be wise to split up the traps into several functions to reduce the large stack frame
 * required due to the large number of local variables.
 */

/* Before this function is called, the caller must ensure that either 
 * the saved java frame registers (in the local register scenario) or the global java frame registers
 * (in the global register scenario) are set to correspond to the location in the java bytecode.
 *
 * When starting a new thread, the location of the first bytecode to execute is the first bytecode in the thread's run method.
 *
 * In the hardware trap scenario, the location of the first bytecode to execute is the byte code pointed to by the native instruction pointer.
 *
 */
void executeCode()
{
    byte opsArray[5];
    byte instruction;
    RETURN_CODE ret;
    CLASS_INSTANCE pClassInstance;
    LOADED_CLASS_DEF pClassDef;
    COMMON_CLASS_DEF pCommonClassDef;
    CLINIT_DATA pClinitData;

    METHOD_DEF pMethodDef;
    INTERFACE_METHOD_DEF pInterfaceMethodDef;
#if !EARLY_NATIVE_LINKING
    NATIVE_METHOD_DEF pNativeMethodDef;
#endif
    FIELD_DEF pFieldDef;
    classIdentifierStruct classID;
    char arrayTypeChars[2] = "[";

    OBJECT_INSTANCE pObject;
    INTERFACE_LIST pInterfaceList;
    
    throwRegisterStruct throwRegisters;
    MONITOR pMonitor;
    MONITOR *ppMonitor;

    NamePackage namePackage;
    NameType nameType;
    UINT16 index;
    UINT16 index2;
    UINT8 instructionSize;

    jlong longValue, longValue2;
#if IMPLEMENTS_FLOAT
    jdouble doubleValue;
    jfloat floatValue;
#endif
    jint intValue;
    jint i;
    jstackField fieldValue, fieldValue2, fieldValue3, fieldValue4;
    JSTACK_FIELD pFieldValue;

    INT32 value1;
    INT32 value2;


    
    STRING_INSTANCE pStringInstance;
#if !USING_GC_STACK_MAPS
    INSTRUCTION pNextInstruction;
#endif

#if LOCAL_FRAME_REGISTERS
    frameRegisterStruct localFrameRegisters = getCurrentJavaThread()->savedJavaRegisters;
#if PREEMPTIVE_THREADS
    getCurrentJavaThread()->pFrameRegisterAccessor = &localFrameRegisters;
#endif
    pFrameRegisterAccessor = &localFrameRegisters;
#endif
    
    getCurrentThread()->prepareSwitchIn(getCurrentThread());
    

instructionSwitch:


#if LOGGING
    /*
    {
        byte currentInstruction = *getPC();
        if(currentInstruction != IDLE_OPCODE) {
            LOG_PATH(("%x", currentInstruction));
            
        } 
    }
    // */
    
#endif

    switch(instruction = *getPC()) {
    
#define INSIDE_INSTRUCTION_LOOP
#include "handlers/arithmeticHandlers.c"
#include "handlers/branchHandlers.c"
#include "handlers/conversionHandlers.c"
#include "handlers/fieldHandlers.c"
#include "handlers/invokeHandlers.c"
#include "handlers/localHandlers.c"
#include "handlers/objectHandlers.c"
#include "handlers/specialHandlers.c"
#include "handlers/stackHandlers.c"



START_INSTRUCTION_HANDLER(WIDE)
        incrementProgramCounter();
        switch(instruction = *getPC()) {

#include "handlers/wideHandlers.c"

            default:
                /* unhandled wide instruction, should never happen */
                instructionBreak();
        }
END_HANDLER


        default:
            /* unhandled instruction, should never happen */
            instructionBreak();
    }

}





RETURN_CODE setupJavaThread(OBJECT_INSTANCE pThreadObject, JAVA_THREAD pThread, METHOD_DEF pInitialMethod, UINT16 initialArgCount, JSTACK_FIELD initialArgs, UINT32 initialStackSize)
{
    RETURN_CODE ret;
    JSTACK_FIELD sp;
    FRAME fp;
    stackParamStruct stackParams;
    UINT16 i;
    
    /* initialize the thread structure */

    /* note that when the thread object was created, the object and struct were both initialized to 0 */
    pThread->pThreadInstance = pThreadObject;
    if((ret = allocateStackBase(&pThread->pStackBase, &stackParams, initialStackSize)) != SUCCESS) {
        return ret;
    }
#if !GROW_STACK
    pThread->pStackLimit = stackParams.pLimit;
#endif

    /* create the fake java frame for the 'catch all' method */
    fp = (FRAME) stackParams.pBase;
    setFrameMethod(fp, &firstMethod);
    setFramePreviousFP(fp, NULL);
    setFramePreviousPC(fp, NULL);
#if GROW_STACK
    setFramePreviousSP(fp, NULL);
    setFrameStackLimit(fp, stackParams.pLimit);
#endif

    /* create the java frame for the thread's startup method */

    sp = endOfFrame(fp);
    /* put in the parameters */
    for(i=0; i<initialArgCount; i++, sp++) {
        *sp = initialArgs[i];
    }
    
    /* make space for the locals and construct the rest of the frame */

#if !USING_GC_STACK_MAPS
    /* we must initialize the locals to zero for the garbage collector */
    for(i=0; i < pInitialMethod->localVarCount; i++) {
        setStackFieldObject(sp + i, NULL);
    }
#endif
    fp = (FRAME) (sp + pInitialMethod->localVarCount);
    setFrameMethod(fp, pInitialMethod);
    setFramePreviousFP(fp, (FRAME) stackParams.pBase);
    setFramePreviousPC(fp, firstMethod.pCode);
#if GROW_STACK
    setFramePreviousSP(fp, sp - (initialArgCount + 1));
    setFrameStackLimit(fp, stackParams.pLimit);
#endif
    
    /* initialize the java frame registers for the thread */

    setJavaThreadLP(pThread, sp - initialArgCount);
    setJavaThreadFP(pThread, fp);
    setJavaThreadSP(pThread, getResetSP(fp, isSynchronizedMethod(pInitialMethod)));
    setJavaThreadPC(pThread, pInitialMethod->pCode);  
    setJavaThreadCurrentMethodDef(pThread, getFrameMethod(fp));
    setJavaThreadCurrentClassDef(pThread, getJavaThreadCurrentMethodDef(pThread)->base.pOwningClass);
    setJavaThreadCurrentConstantPool(pThread, getJavaThreadCurrentClassDef(pThread)->loadedHeader.pConstantPool);

    return SUCCESS;
}




RETURN_CODE startThread(OBJECT_INSTANCE pThreadObject, JAVA_THREAD pThread, METHOD_DEF pInitialMethod, MONITOR pMonitor, UINT16 initialArgCount, JSTACK_FIELD initialArgs, UINT32 initialStackSize)
{
    RETURN_CODE ret;
    
    ret = setupJavaThread(pThreadObject, pThread, pInitialMethod, initialArgCount, initialArgs, initialStackSize);
    if(ret != SUCCESS) {
        return ret;
    }

#if PREEMPTIVE_THREADS
    ret = initializeNativeStack(&pThread->header.savedThreadParameters, executeCode, &pThread->header.pNativeStack);
    if(ret != SUCCESS) {
        return ret;
    }
#endif
    
    if(isSynchronizedMethod(pInitialMethod)) {
        setMonitorInFrame(getJavaThreadFP(pThread), pMonitor);
    }

    /* everything is prepared, now schedule it in */
    scheduleNewJavaThread(pThread, pMonitor);
    
    return SUCCESS;
}


/* 
 * these two functions are called whenever a java thread is switched out or switched in.
 * they are responsible for saving and restoring any data that is thread-specific.
 */
void prepareSwitchJavaThreadOut(JAVA_THREAD pThread) 
{
#if LOCAL_FRAME_REGISTERS
#if PREEMPTIVE_THREADS
    /* if the java registers are local, we save the pointer to their
     * location on the current thread's stack 
     */
    pThread->pFrameRegisterAccessor = pFrameRegisterAccessor;
#endif
#else
    /* the java registers are not local, so the current thread's registers
     * must be saved
     */
    pThread->savedJavaRegisters = frameRegisters;
#endif
    //TODO PROCESS MODEL: switch out the string pools and class table
}


void prepareSwitchJavaThreadIn(JAVA_THREAD pThread)
{
#if LOCAL_FRAME_REGISTERS
#if PREEMPTIVE_THREADS
    /* if the java registers are local, we assign the pointer to their
     * location on the new thread's stack 
     */
    pFrameRegisterAccessor = pThread->pFrameRegisterAccessor;
#endif
#else
    /* the java registers are not local, so the current thread's registers
     * must be restored into the global registers
     */
    frameRegisters = pThread->savedJavaRegisters;
#endif

    //TODO PROCESS MODEL: switch in the string pools and class table
}



RETURN_CODE startNewThread(OBJECT_INSTANCE pObject, UINT32 initialStackSize)
{
    THREAD_INSTANCE pThreadObject = getThreadInstanceFromObjectInstance(pObject);
    JAVA_THREAD pThread; 
    RETURN_CODE ret;
    METHOD_DEF pRunMethod;
    jstackField arg;
    MONITOR pMonitor;

    ret = createThreadStruct(pThreadObject);
    if(ret != SUCCESS) {
        return ret;
    }
    pThread = getThreadStructFromThreadObject(pThreadObject);
#if PREEMPTIVE_THREADS
    initializeJavaThreadStruct(pThread, prepareSwitchJavaThreadOut, prepareSwitchJavaThreadIn, cleanUpNativeStack, THREAD_DEFAULT_PRIORITY);
#else
    initializeJavaThreadStruct(pThread, prepareSwitchJavaThreadOut, prepareSwitchJavaThreadIn, NULL, THREAD_DEFAULT_PRIORITY);
#endif
    /* note that this thread's class could be a subclass of java.lang.Thread so we get the class def from the object */
    ret = lookupMethod((LOADED_CLASS_DEF) pThreadObject->header.pClassDef, runNameType, TRUE, &pRunMethod);
    if(ret != SUCCESS) {
        return ret;
    }
    insertStackFieldObject(arg, pObject);

    if(isSynchronizedMethod(pRunMethod)) {
        ret = collectibleMemoryCallocMonitor(getObjectMonitorPtr(pObject));
        if(ret != SUCCESS) {
            return ret;
        }
        pMonitor = getObjectMonitor(pObject);
    }
    else {
        pMonitor = NULL;
    }

    return startThread(pObject, pThread, pRunMethod, pMonitor, 1, &arg, initialStackSize);
}




