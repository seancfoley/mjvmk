
#include "thread/schedule.h"
#include "operate/log.h"
#include "operate/operate.h"

#if PREEMPTIVE_THREADS


static int counter = 0;
semaphoreStruct sem = {2, NULL};
    
void printSemStatement()
{
    char str[500];
    stringPrintf((str, "%p is in semaphore\n", getCurrentThread()));
    psPrintOut(str);   
}

void printSemStatement2()
{
    char str[500];
    stringPrintf((str, "%p is out of semaphore\n", getCurrentThread()));
    psPrintOut(str);   
}

void task4()
{
    timeValueStruct semPauseTime;
    setMillis(&semPauseTime, 500);
    
    while(TRUE) {
        semaphorePend(&sem);
        printSemStatement();
        threadSleep(&semPauseTime);
        printSemStatement2();
        semaphorePost(&sem);
    }
}

void task5()
{
    timeValueStruct semPauseTime;
    setMillis(&semPauseTime, 500);
    
    while(TRUE) {
        semaphorePend(&sem);
        printSemStatement();
        threadSleep(&semPauseTime);
        printSemStatement2();
        semaphorePost(&sem);
    }
}

void task6()
{
    timeValueStruct semPauseTime;
    setMillis(&semPauseTime, 500);
    
    while(TRUE) {
        semaphorePend(&sem);
        printSemStatement();
        threadSleep(&semPauseTime);
        printSemStatement2();
        semaphorePost(&sem);
    }
}

void task1()
{
    timeValueStruct pauseTime;
    
    setMillis(&pauseTime, 100);
    //setPriority(getCurrentThread(), 33);

    while(TRUE) {
        psPrintOut("Task 1\n");
        threadSleep(&pauseTime);
        if(counter++ >= 10) {
            while(TRUE);
        }
    }
}

void task2()
{
    timeValueStruct pauseTime;
    
    setMillis(&pauseTime, 200);
    
    while(TRUE) {
        psPrintOut("Task 2\n");
        threadSleep(&pauseTime);
        if(counter++ >= 10) {
            while(TRUE);
        }
    }
}

void task3()
{
    psPrintOut("Task 3\n");

    /* let's remove ourself just for fun */
    removeThread();
    
}

THREAD pThread1, pThread2, pThread3, pThread4, pThread5, pThread6;

void startTasks()
{
    /* At this point we are the idle task.  So the last thing we do is call idleFunction.
     * But before that we create any additional tasks.
     */

    /* create your tasks, threads, apps here
    {
        THREAD pAppThread;
        startNativeThread(pTaskFunction, pSwitchOutFunction, pSwitchInFunction, pMonitor, THREAD_DEFAULT_PRIORITY, &pAppThread);
    }
    */
    {     
        startNativeThread(task1, NULL, NULL, NULL, THREAD_DEFAULT_PRIORITY, &pThread1);
        startNativeThread(task2, NULL, NULL, NULL, THREAD_DEFAULT_PRIORITY, &pThread2);
        startNativeThread(task3, NULL, NULL, NULL, THREAD_DEFAULT_PRIORITY, &pThread3);
        startNativeThread(task4, NULL, NULL, NULL, THREAD_DEFAULT_PRIORITY, &pThread4);
        startNativeThread(task5, NULL, NULL, NULL, THREAD_DEFAULT_PRIORITY, &pThread5);
        startNativeThread(task6, NULL, NULL, NULL, THREAD_DEFAULT_PRIORITY, &pThread6);
        
    }


#if THREADED_GC
    startNativeThread(garbageCollectionFunction, NULL, NULL, NULL, THREAD_GC_PRIORITY, &pGCThread);
#endif
    startTick();
    idleFunction();
}

#endif

/* The start function is the first function executed.  After boot-up the system jumps
 * to start() in some configurations
 * 
 */
void start()
{
    if(initializeOS() != SC_SUCCESS) {
        return;
    }

#if PREEMPTIVE_THREADS
    /* Note: we need not become the idle thread before creating any other threads/tasks,
     * but if we create tasks first then we must ensure that the stack currently being used
     * is not overwritten, if it has become part of the heap.
     *
     * Functions like startNativeThread obtain and write to memory from the heap so if the
     * current stack is part of the heap, it is safer to create the idle thread first, discarding
     * the current stack for the idle thread's stack.
     */
    
    
    /* we will return only if there has been an error, otherwise we will jump to startTasks 
     * as the idle thread.
     */
    becomeIdleThread(startTasks);
    LOG_LINE(("initialization error: error becoming idle thread"));
    psPrintErr("initialization error: error becoming idle thread");
#else
    /* your start function goes here for a non-kernel threaded O/S */
#endif
}

/* in some cases main is required if we use the standard C library */
int main()
{
    start();
    return 0;
}
