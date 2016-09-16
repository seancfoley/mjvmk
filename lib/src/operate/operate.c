
#include "types/config.h"
#include "thread/schedule.h"
#include "memory/heap.h"
#include "memory/garbageCollect.h"
#include "log.h"
#include "operate.h"

#if PREEMPTIVE_THREADS


void idleFunction()
{
    while(TRUE) {
	
        /* check for timed-out threads */
    
        /* yield control to anybody else who wants to run */
    
        strongYield();

#if IDLE_GC
        /* use this free time to collect garbage */
    
        garbageCollect();

        /* not that we've collected garbage, try finding another thread again */

	    strongYield();
#endif

        /* go to the platform specific power conversion function */

        psSleep();
	    
	    /* Here the sleep was interrupted by an interrupt or some other external cue
         * indicating that the device should awaken, or there is no platform specific sleep functionality.  
         * Note that the tick interrupt must disrupt the sleep, 
         * so that any threads waiting for timeouts are moved off the timer queue and become ready to run.
         */

        /* Note: for instance you could have an event handling thread
         * waiting on a monitor - a sleep disruption would then notify that monitor in the ISR.
         */
    }
}

#if THREADED_GC

THREAD pGCThread;
monitorStruct threadedGCMonitor = {0, NULL, NULL, NULL};

void garbageCollectionFunction()
{
    timeValueStruct delay;
    setMillis(&delay, GC_DELAY);
    
    while(TRUE) {
        garbageCollect();
        strongYield();
        
        /* we sleep for a specific amount of time for the idle thread to kick in and 
         * power conservation to take place 
         */
        threadSleep(&delay);
    }
    
}

#endif

void cleanUpNativeStack(THREAD pThread)
{
    /* once this function is called, if and while the thread's native
     * stack continues to be used we must be certain that:
     * - we are in a critical region
     * - we will not write to any memory subsequently obtained from the heap
     *
     * Otherwise the thread's native stack and memory obtained from the heap might become corrupted.
     */
    memoryFree(pThread->pNativeStack);
}

SUCCESS_CODE initializeNativeStack(PS_STACK_PARAMS pThreadParameters, void (*pTask)(), PS_STACK_ELEMENT *ppStack)
{
    PS_STACK_ELEMENT pNativeStack = memoryAlloc(NATIVE_STACK_SIZE * sizeof(stackElement));

    if(pNativeStack == NULL) {
        return SC_FAILURE;
    }
    *ppStack = pNativeStack;
    initializeNativeStackParameters(pThreadParameters, pNativeStack, NATIVE_STACK_SIZE, pTask);
    return SC_SUCCESS;
}

/* if provide a monitor, the thread will enter the monitor before execution begins,
 * and will not run until that monitor becomes available.
 * Otherwise the thread will be added to the list of threads available to run.
 */
SUCCESS_CODE startNativeThread(void (*pTask)(), void (*pSwitchOut)(THREAD), void (*pSwitchIn)(THREAD), MONITOR pMonitor, UINT8 priority, THREAD *ppThread)
{
    SUCCESS_CODE ret;

    THREAD pThread = memoryAlloc(sizeof(threadStruct));
    if(pThread == NULL) {
        return SC_FAILURE;
    }
    
    ret = initializeNativeStack(&pThread->savedThreadParameters, pTask, &pThread->pNativeStack);
    if(ret != SC_SUCCESS) {
        return ret;
    }

    initializeThreadStruct(pThread, pSwitchOut, pSwitchIn, cleanUpNativeStack, priority);

    scheduleNewThread(pThread, pMonitor);

    *ppThread = pThread;
    return SC_SUCCESS;
}

SUCCESS_CODE initializeIdleStack(void (*pReturnAddress)())
{
    /* the current thread, which will become the idle thread, needs
     * a properly allocated native stack, now that we have a heap.
     * The current stack is only temporary and could be corrupted as soon
     * as we write to any memory obtained from the heap.
     *
     * the idle thread's stack size needs to be large enough to run pReturnAddress
     */
    PS_STACK_ELEMENT pNativeStack = memoryAlloc(IDLE_STACK_SIZE * sizeof(stackElement));
    
    if(pNativeStack == NULL) {
        LOG_LINE(("initialization error: error allocating native stack"));
        psPrintErr("initialization error: error allocating native stack");
        return SC_FAILURE;
    }
    intializeCurrentThreadNativeStack(pNativeStack, IDLE_STACK_SIZE, pReturnAddress);
    
    /* we will never reach here, instead we will jump directly to pReturnAddress */

    return SC_SUCCESS;
}

threadStruct idleThread;

/* there cannot be any other threads in the scheduler when this is called */
SUCCESS_CODE becomeIdleThread(void (*pReturnAddress)())
{
    /* Note: if the memory being used by the current stack has become part of the
     * heap (which is recommended to avoid waste) then we CANNOT write to any memory
     * obtained from the heap until we initialize the idle stack by calling initializeIdleStack, 
     * otherwise the current stack could become corrupted.
     */

    pIdleThread = &idleThread;
    
    /* Note: since the idle thread never dies, we need not concern ourselves with a thread cleanup function */
    initializeThreadStruct(pIdleThread, NULL, NULL, NULL, THREAD_IDLE_PRIORITY);

    setCurrentThread(pIdleThread);

    /* there cannot be any other threads in the scheduler when this is called, otherwise
     * the following function call will cause a context switch 
     */
    scheduleNewThread(pIdleThread, NULL);

    /* we will not return unless there has been an error, 
     * instead we will have jumped to pReturnAddress
     */
    return initializeIdleStack(pReturnAddress);
}


#endif


/* this function is the main initialization function, and is independent of the
 * threading configuration.  To be called regardless of the setting of PREEMPTIVE_THREADS
 * to initialize logging, the heap, CPU, etc...
 */
SUCCESS_CODE initializeOS()
{
    psPrintBanner();

#if LOGGING
    if(initializeLogging() != SC_SUCCESS) {
        psPrintErr("initialization error: error initializing logging\n");
        return SC_FAILURE;
    }
#endif

    LOG_LINE(("initializing CPU"));
    if(initializeCPU() != SC_SUCCESS) {
        LOG_LINE(("initialization error: error initializing CPU"));
        psPrintErr("initialization error: error initializing CPU");
        return SC_FAILURE;
    }

    LOG_LINE(("initializing heap"));
    if(initializeHeap() != SC_SUCCESS) {
        LOG_LINE(("initialization error: error initializing heap"));
        psPrintErr("initialization error: error initializing heap");
        return SC_FAILURE;
    }

    return SC_SUCCESS;
}


