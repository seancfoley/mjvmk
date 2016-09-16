#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "thread.h"



#if USE_TIME_SHARES

/* Time slice shares */


/* all threads running at the same priority will garner different shares of the CPU time
 * depending upon their time share value.
 */

/* Whenever a thread is switched out its counter is restored to 0.
 * When a thread is selected its counter is incremented by its own time share value.
 * If the counter is high enough the thread will run.  When the thread completes its full cycle
 * of allotted CPU time (typically the interval between clock ticks), its timeShareCounter is decremented.  
 * If the thread's time share value is still high enough it continues running for another cycle,
 * and so on until its time shares run out.
 *
 * See timeShareEnum for more details.
 */


    

/* 
 * We increment each thread's time share counter until we find a thread
 * with a time share high enough to become the running thread.
 */
#define selectRunningThread() {                                                 \
    THREAD pRunningThread;                                                      \
                                                                                \
    do {                                                                        \
        pRunningThread = rotateRunningThreadList()->pThread;                    \
        pRunningThread->timeShareCounter += pRunningThread->timeShareAllotment; \
    } while(pRunningThread->timeShareCounter < TIME_SHARE_REGULAR);             \
}

#define timeShareCounterExpired() (getCurrentThread()->timeShareCounter == 0)

#define deductTimeShare() (getCurrentThread()->timeShareCounter -= TIME_SHARE_REGULAR)





/* a thread was switched out from the running list, 
 * so we start with the first thread on the list 
 */

#define selectNextRunningThread() {                                             \
    THREAD_LIST pThreadList = getCurrentRunningThreadList();                    \
    THREAD pRunningThread;                                                      \
                                                                                \
    if(pThreadList) {                                                           \
        pRunningThread = pThreadList->pThread;                                  \
        pRunningThread->timeShareCounter += pRunningThread->timeShareAllotment; \
        while(pRunningThread->timeShareCounter < TIME_SHARE_REGULAR) {          \
            pRunningThread = rotateRunningThreadList()->pThread;                \
            pRunningThread->timeShareCounter +=                                 \
                pRunningThread->timeShareAllotment;                             \
        }                                                                       \
    }                                                                           \
}

/* the current thread is not longer in the running thread list, it has been
 * moved elsewhere, so we reset its timeShareCounter and then find another thread 
 */
#define resetTimeShare() {                                                      \
    getCurrentThread()->timeShareCounter = 0;                                   \
    selectNextRunningThread();                                                  \
}


#else

/* the current thread has been switched out of the running threads */
#define resetTimeShare()

#endif





/* Macros for moving threads from one list to the next */


/* 
 * add the thread to the list: be sure to remove it from whatever list 
 * it is already in first - we add to the end for a FIFO list
 */
#define threadListAddThread(ppDestList, pThread)                                            \
    doubleListAddOneToEnd((LINKED_LIST *) (ppDestList), (LINKED_LIST) &(pThread)->listStruct)

/* removes the specified thread from the list, be sure it is actually on the specified list */
#define threadListRemoveThread(ppList, pThread)                                             \
    doubleListRemoveOne((LINKED_LIST *) ((THREAD_LIST *) ppList),                           \
        (LINKED_LIST) &(((THREAD) pThread)->listStruct))

/* moves all of the source list to the end of the dest list */
#define threadListMoveAll(ppDestList, ppSourceList)                                         \
    doubleListMoveAllToEnd((LINKED_LIST *) (ppDestList), (LINKED_LIST *) (ppSourceList))

/* moves the front entry from the source list to the end of the dest list */
#define threadListMoveOne(ppDestList, ppSourceList)                                         \
    doubleListMoveOneFrontToEnd((LINKED_LIST *) (ppDestList), (LINKED_LIST *) (ppSourceList))

/* rotates one from the front to the end */
#define threadListRotate(ppList)                                                            \
    ((THREAD_LIST) doubleListRotateOneFrontToEnd((LINKED_LIST *) (ppList)))

/* removes one from front */
#define threadListRemoveOne(ppList)                                                         \
    ((THREAD_LIST) doubleListRemoveOneFromFront((LINKED_LIST *) (ppList)))

/* moves a thread onto a list sorted by priority */
#define addThreadToSortedQueue(ppThreadList, pThread, comparator)                           \
    doubleListAddOneSorted((LINKED_LIST *) (ppThreadList),                                  \
        (LINKED_LIST) &(pThread)->listStruct,                                               \
        (BOOLEAN (* )(LINKED_LIST, LINKED_LIST)) comparator)


#define ITERATE_THREAD_LIST(pThreadList) ITERATE_LIST((LINKED_LIST) (pThreadList))
#define END_ITERATE_THREAD_LIST(pThreadList) END_ITERATE_LIST((LINKED_LIST) (pThreadList))




/* Thread priorities */


/* Obviously the idle thread should have the lowest priority. */

/* a thread at the inactive priority will never be given CPU time */
#define THREAD_INACTIVE_PRIORITY 0 
#define THREAD_IDLE_PRIORITY 1
#define THREAD_GC_PRIORITY THREAD_DEFAULT_PRIORITY
#define THREAD_DEFAULT_PRIORITY 31
#define THREAD_MAX_PRIORITY 63


extern UINT8 priorityMatrix[8];
extern UINT8 priorityRows;
extern UINT8 highestBitMap[];


#define LOWEST3BITSMASK 0x07

#define addToPriorityMap(priority)                                              \
    priorityRows |= (1 << (priority >> 3));                                     \
    priorityMatrix[priority >> 3] |= (1 << (priority & LOWEST3BITSMASK))

#define removeFromPriorityMap(priority)                                         \
    priorityMatrix[priority >> 3] &= ~(1 << (priority & LOWEST3BITSMASK));      \
    if(priorityMatrix[priority >> 3] == 0)                                      \
        priorityRows &= ~(1 << (priority >> 3))



#define getHighestPriority()                                                    \
    ((highestBitMap[priorityRows] << 3) +                                       \
        highestBitMap[priorityMatrix[highestBitMap[priorityRows]]])


extern THREAD_LIST runningThreads[64];

#define getHighestPriorityThread() (runningThreads[getHighestPriority()]->pThread)
#define getCurrentRunningThreadList() (runningThreads[getCurrentThread()->currentPriority])
#define rotateRunningThreadList() threadListRotate(runningThreads + getCurrentThread()->currentPriority)

void monitorEnterBlockedMonitor(MONITOR pMonitor);
void monitorExitContendedMonitor(MONITOR pMonitor);
void setThreadPriority(THREAD pThread, UINT8 priority);
void removeThreadCritical();
#define getThreadPriority(pThread) (pThread->currentPriority)


/* Threading API */

void scheduleNewThread(THREAD pNewThread, MONITOR pMonitor);
void removeThread();
void interruptThread(THREAD pThread);
UINT32 getActiveThreadCount();
void setPriority(THREAD pThread, UINT8 priority);
void threadSleep(ELAPSED_TIME pTimeOut);
void threadJoin(THREAD pThreadToJoin);
void weakYield();
void strongYield();

#if PREEMPTIVE_THREADS

/* pairs of these calls may be nested, it is the outermost pair that have any effect */
void disableRescheduler();
void enableRescheduler();
extern UINT8 reschedulerEnabledIndicator;

#else

/* rescheduling is handled explicitly with non-preemptive threads */
#define disableRescheduler()
#define enableRescheduler()

#endif



/* monitor operations
 *
 * These monitors have condition variables.  
 * Monitors must be entered before any wait or notify operations are allowed.  
 * It is possible for a thread running inside a monitor to suspend its execution
 * so that another thread may enter the monitor. 
 * The initial thread waits for the second one to notify it and then to exit the monitor.
 */

/* intializing a monitor simply zeroes the monitor, 
 * which means that you need not initialize global monitors 
 */
#define monitorInit(pMonitor) memorySet(pMonitor, 0, sizeof(monitorStruct))

#define monitorEnter(pMonitor) {                        \
    enterCritical();                                    \
    if((pMonitor)->pOwner == NULL) {                    \
        (pMonitor)->pOwner = getCurrentThread();        \
        (pMonitor)->depth = 1;                          \
    }                                                   \
    else if(pMonitor->pOwner == getCurrentThread()) {   \
        (pMonitor)->depth++;                            \
    }                                                   \
    else {                                              \
        monitorEnterBlockedMonitor(pMonitor);           \
    }                                                   \
    exitCritical();                                     \
}

#define monitorExit(pMonitor) {                         \
    enterCritical();                                    \
    if(--(pMonitor)->depth == 0) {                      \
        if((pMonitor)->pBlockedList != NULL) {          \
            monitorExitContendedMonitor(pMonitor);      \
        }                                               \
        else {                                          \
            (pMonitor)->pOwner = NULL;                  \
        }                                               \
    }                                                   \
    exitCritical();                                     \
}

void monitorWait(MONITOR pMonitor, ELAPSED_TIME pTimeOut);
void monitorNotify(MONITOR pMonitor);
void monitorNotifyAll(MONITOR pMonitor);



/* semaphore operations */

#define semaphoreInit(pSemaphore, initialValue) {                               \
    (pSemaphore)->counter = initialValue;                                       \
    (pSemaphore)->pBlockedList = NULL;                                          \
}

void semaphorePost(SEMAPHORE pSemaphore);
void semaphorePend(SEMAPHORE pSemaphore);

#if HIGHEST_PRIORITY_SEMAPHORES

BOOLEAN threadPriorityComparator(THREAD_LIST a, THREAD_LIST b);

#define moveCurrentThreadToSemaphoreQueue(pSemaphore) {                         \
    removeCurrentThreadFromRunningThreads();                                    \
    addThreadToSortedQueue(&(pSemaphore)->pBlockedList, getCurrentThread(),     \
        threadPriorityComparator);                                              \
}

#else

#define moveCurrentThreadToSemaphoreQueue(pSemaphore)                           \
    moveCurrentThreadFromRunningThreads(&(pSemaphore)->pBlockedList)

#endif




#endif
