
#include "memory/memoryAlloc.h"
#include "memory/garbageCollect.h"
#include "operate/operate.h"
#include "thread/schedule.h"
#include "javaThread/javaThread.h"
#include "object/allocate.h"
#include "object/instantiate.h"
#include "string/javaString.h"
#include "string/stringKeys.h"
#include "log/logItem.h"
#include "class/table.h"
#include "resolution/file/fileAccess.h"
#include "resolution/load/initializeLoader.h"
#include "resolution/resolve/resolve.h"
#include "resolution/resolve/lookup.h"
#include "resolution/resource.h"
#include "natives/initializeNatives.h"
#include "execute/execute.h"
#include "run.h"


#define outputInfo(ret) {                               \
    char out[10];                                       \
    LOG_LINE(("initialization error: %d", ret));        \
    psPrintErr("initialization error: ");               \
    integerToString(ret, out, 10);                      \
    psPrintErr(out);                                    \
    psPrintErr("\n");                                   \
}

#define handleInitializer(initFunction) {               \
    if((ret = (initFunction)) != SUCCESS) {             \
        outputInfo(ret);                                \
        return ret;                                     \
    }                                                   \
}


static RETURN_CODE initializeCurrentThreadAsIdleThread()
{
    OBJECT_INSTANCE pObject;
    THREAD_INSTANCE pThreadObject;
    RETURN_CODE ret;
    JAVA_THREAD pThread;
    
    /* don't bother initializing this thread object, the object itself we will never need */
    handleInitializer(instantiatePermanentObject(pJavaLangThread, &pObject));
    pThreadObject = getThreadInstanceFromObjectInstance(pObject);
    handleInitializer(createThreadStruct(pThreadObject));
    
    pThread = getThreadStructFromThreadObject(pThreadObject);

    /* Note: since the idle thread never dies, we need not concern ourselves with a thread cleanup function */
    initializeJavaThreadStruct(pThread, prepareSwitchJavaThreadOut, prepareSwitchJavaThreadIn, NULL, THREAD_IDLE_PRIORITY);

    /* we calculate the stack size as follows:
     * - we need a frame for the initial first stack frame at the base of all threads, this frame has no locals, no op stack and is not synchronized
     * - we need a frame for the idle method which has no args, no op stack and is not synchronized
     */
    handleInitializer(setupJavaThread(pObject, pThread, pIdleMethod, 0, NULL, 2 * getInvokeSize(0, 0, FALSE)));
    
    /* There is no need to create and initialize the thread's native stack, we already have one, the
     * one being used at this very moment.
     */
    setCurrentThread((THREAD) pThread);
    scheduleNewJavaThread(pThread, NULL);
    pIdleThread = (THREAD) pThread;
    return SUCCESS;
}


/* Note: there is no need to be careful with collectible objects since garbage collection has not
 * yet begun.
 */
static RETURN_CODE parseArgs(char *args, UINT16 startArgsLen, ARRAY_INSTANCE *ppArrayInstance)
{
    UINT16 count = 0;
    UINT16 i;
    char *cp1;
    char *cp2;
    INTERNED_STRING_INSTANCE pFirstString = NULL;
    INTERNED_STRING_INSTANCE pString = NULL;
    INTERNED_STRING_INSTANCE pPreviousString;
    OBJECT_INSTANCE pObject;
    RETURN_CODE ret;
    ARRAY_INSTANCE pRes;

    cp1 = args;
    while(TRUE) {
        count++;
        cp2 = stringChar(cp1, ' ');
        if(cp2 != NULL) {
            *cp2 = '\0';
            pPreviousString = pString;
            handleInitializer(instantiateInternedString(cp1, (UINT16) (cp2 - cp1), &pObject));
            pString = getInternedStringInstanceFromObjectInstance(pObject);
            if(pPreviousString) {
                pPreviousString->next = pString;
            }
            else {
                pFirstString = pString;
            }
            cp2++;
            while(*cp2 == ' ') cp2++;
            startArgsLen -= cp2 - cp1;
        }
        else {
            cp2 = cp1 + startArgsLen;
            pPreviousString = pString;
            handleInitializer(instantiateInternedString(cp1, startArgsLen, &pObject));
            pString = getInternedStringInstanceFromObjectInstance(pObject);
            if(pPreviousString) {
                pPreviousString->next = pString;
            }
            else {
                pFirstString = pString;
            }
            break;
        }
        cp1 = cp2;
    }

    handleInitializer(instantiateCollectibleObjectArray(pJavaLangStringArrayClass, count, &pRes));
    *ppArrayInstance = pRes;
    for(i=0, pString = pFirstString; i<count; pString = pString->next, i++) {
        setObjectArrayElement(pRes, i, (OBJECT_INSTANCE) &pString->header);    
    }

    return SUCCESS;
}


static RETURN_CODE startMainJavaThread(UINT32 initialStackSize)
{
    RETURN_CODE ret;
    UINT16 i;
    UINT16 startClassLen;
    UINT16 startArgsLen;
    char line[MAX_PROPERTY_LINE_LEN];
    char line2[MAX_PROPERTY_LINE_LEN];
    char *startClass;
    char *startArgs;
    classIdentifierStruct classID;
    CLASS_INSTANCE pClassInstance;
    OBJECT_INSTANCE pThreadObject;
    METHOD_DEF pMainMethod;
    JAVA_THREAD pThread;
    ARRAY_INSTANCE startArgsArray;
    jstackField arg;
    MONITOR pMonitor;
    
    ret = getProperty(START_CLASS_PROPERTY_KEY, &startClassLen, line, &startClass);
    if(ret != SUCCESS) {
        return ret;
    }
    if(startClass == NULL) {
        return ERROR_CODE_NO_CLASS_DEF_FOUND;
    }
    for(i=0; i<startClassLen; i++) {
        if(startClass[i] == '.') startClass[i] = '/';
    }

    ret = getProperty(START_ARGS_PROPERTY_KEY, &startArgsLen, line2, &startArgs);
    if(ret != SUCCESS) {
        return ret;
    }
    if(startArgs == NULL) {
        startArgsArray = NULL;
    }
    else {
         handleInitializer(parseArgs(startArgs, startArgsLen, &startArgsArray));
    }
    insertStackFieldObject(arg, (OBJECT_INSTANCE) startArgsArray);

    handleInitializer(getClassParams(startClass, startClassLen, &classID));

    ret = obtainClassRef(classID.key, &pClassInstance);
    if(ret != SUCCESS) {
        psPrintErr("Could not locate start class\n");
        outputInfo(ret);
        return -1;
    }

    handleInitializer(instantiateCollectibleObject(pJavaLangThread, &pThreadObject));
    
    handleInitializer(createThreadStruct(getThreadInstanceFromObjectInstance(pThreadObject)));
    pThread = getThreadStructFromObject(pThreadObject);
#if PREEMPTIVE_THREADS    
    initializeJavaThreadStruct(pThread, prepareSwitchJavaThreadOut, prepareSwitchJavaThreadIn, cleanUpNativeStack, THREAD_DEFAULT_PRIORITY);
#else
    initializeJavaThreadStruct(pThread, prepareSwitchJavaThreadOut, prepareSwitchJavaThreadIn, NULL, THREAD_DEFAULT_PRIORITY);
#endif
    ret = lookupStaticMethod((LOADED_CLASS_DEF) (pClassInstance->pRepresentativeClassDef), mainNameType, &pMainMethod);
    if(ret != SUCCESS) {
        psPrintErr("Could not locate method ");
        psPrintErr(mainString.string);
        psPrintErr(" ");
        psPrintErr(aJavaLangStringVType.string);
        psPrintErr(" in start class\n");
        outputInfo(ret);
        return -1;
    }

    if(isSynchronizedMethod(pMainMethod)) {
        pMonitor = pClassInstance->instance.pMonitor;
        if(pMonitor == NULL) {
            handleInitializer(memoryCallocMonitor(getClassMonitorPtr(pClassInstance)));
        }
    }
    else {
        pMonitor = NULL;
    }

    handleInitializer(startThread(pThreadObject, pThread, pMainMethod, pMonitor, 1, &arg, initialStackSize));
    
    if(!classIsInitialized(pClassInstance, pThread)) {
        
#if GROW_STACK
        JSTACK_FIELD pStackLimit = getFrameStackLimit(getJavaThreadFP(pThread));
#else
        JSTACK_FIELD pStackLimit = pThread->pStackLimit;
#endif
        
        initializeClassAtStartUp(pThread, pClassInstance, pStackLimit, return);
    }
    
    return SUCCESS;
}



RETURN_CODE initializeVM()
{
    RETURN_CODE ret;
    char *classPath = COMPILE_TIME_CLASS_PATH;

    /* the order below is important */

    LOG_LINE(("VM starting up"));

    LOG_LINE(("initializing string pools"));
    handleInitializer(initializeStringPools());
    
    initializeGlobalStrings();

    LOG_LINE(("initializing class table"));
    handleInitializer(initializeClassTable());
    
    LOG_LINE(("initializing native table"));
    handleInitializer(initializeNativeTable());

    LOG_LINE(("initializing class path"));
    handleInitializer(initializeFileAccess(classPath));

    LOG_LINE(("initializing loader"));
    handleInitializer(initializeLoader()); 
    
    LOG_LINE(("initializing execution module"));
    initializeExecution();
    
    LOG_LINE(("initializing idle thread"));
    handleInitializer(initializeCurrentThreadAsIdleThread());
    
    LOG_LINE(("initializing start-up thread"));
    handleInitializer(startMainJavaThread(INITIAL_JAVA_STACK_SIZE));
    
    LOG_LINE(("initializing garbage collection"));
    setGCMarkFunction(javaMarkFunction);

    
#if PREEMPTIVE_THREADS 
#if THREADED_GC

    LOG_LINE(("starting garbage collection thread"));
    handleInitializer(startNativeThread(garbageCollectionFunction, NULL, NULL, NULL, THREAD_GC_PRIORITY, &pGCThread));

#endif
#else
    /* here is where a garbage collection thread would go int
     * the non-preemptive VM.
     * To implement this we would need
     * a garbage collection opcode.
     */
#endif

    LOG_LINE(("initialization complete"));

    return SUCCESS;
}


void initializeRuntime()
{
    RETURN_CODE ret;

    ret = initializeVM();
    if(ret != SUCCESS) {
        return;
    }

    /* we cannot use the scheduler until it has been supplied with at least one thread.
     * Fortunately, it was supplied with two threads, 
     * the idle thread and the main thread created in the VM initialization.
     */
#if PREEMPTIVE_THREADS
    startTick();
#endif
    
    executeCode();
}


#if TESTING
#include "test.c"
#endif


/* The start function is the first function executed.  After boot-up the system jumps
 * to start()
 * 
 * Here we set up the idle thread as a java thread (we do not call becomeIdleThread),
 * which ensures that the VM's threading remains relatively the same whether we use 
 * preemptive java threads or not.
 */
void start()
{
    if(initializeOS() != SC_SUCCESS) {
        return;
    }
#if TESTING
    //return test(argc, argv);
#endif

#if PREEMPTIVE_THREADS
    if(initializeIdleStack(initializeRuntime) != SC_SUCCESS) {
        return;
    }
#else
    initializeRuntime();
#endif

    return;
}





