
#include "contextSwitch.h"
#include "schedule.h"





/*
 * The scheduler works as follows:
 *
 * There are 64 lists of running threads, each corresponding to a particular priority.
 * The system attempts to run the highest priority thread at all times.  If there are multiple
 * threads at the highest priority, they are run in a round-robin basis.  
 *
 * This round-robin basis can be altered by optionally giving threads weighted time shares: 
 * see the description of the parameter USE_TIME_SHARES for details.
 *
 * The scheduler will move threads around the 64 running lists and various wait and sleep 
 * lists as requested by the scheduler API.  For a description of these various lists, see
 * the description of the listStruct member of the structure threadStruct in thread.h.
 * The scheduling API is similar to that provided by the java programming language.
 *
 * When a scheduler operation is completed, a thread switch is required if the current thread 
 * as identified by the current thread register is not the same as the thread at the front of 
 * the highest priority thread list (known as the highest priority thread).
 * 
 * If the threads are PREEMPTIVE_THREADS (they are at the kernel level), then the scheduler will
 * handler the required context switching when a thread switch is required and reset
 * the current thread register.  
 * 
 * If threads are not PREEMPTIVE_THREADS (they are at the user level), 
 * it is up to the caller of the API to detect when a thread switch is required by checking 
 * if the thread identified by the current thread register is the same as the highest priority 
 * thread.  The caller is then responsible to do any necessary switching and
 * to reset the current thread register.
 */

/* Data structures and macros for determining the running thread with highest priority */

/* priorityMatrix is an 8X8 bit matrix of priorities, from 0 to 63.
 * Bit x in the matrix is 1 if and only if there exists a running
 * thread at that priority.
 */
UINT8 priorityMatrix[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* priorityRows is another bit map.  Bit x is 1 if and only if
 * row x in the priorityMatrix is non-zero.
 */
UINT8 priorityRows = 0;

/* 
 * This array gives the number indicating the highest 
 * significant bit in a given unsigned 8-bit integer
 */
UINT8 highestBitMap[] = {
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};


/* Global lists */

/* TL5: the active thread at a given priority is at the front of its list */

THREAD_LIST runningThreads[64];     

static THREAD_LIST asleepList = NULL;             /* TL1: this list is unordered */

/* the active thread list */

static UINT32 activeThreadCount = 0;
static THREAD_LIST activeThreadList = NULL;       /* all active threads are in this list */

#define activeThreadListRemoveThread(pThread)                                                   \
    doubleListRemoveOne((LINKED_LIST *) &activeThreadList,                                      \
        (LINKED_LIST) &(pThread)->activeListStruct)

#define activeThreadListAddThread(pThread)                                                      \
    doubleListAddOneToEnd((LINKED_LIST *) &activeThreadList,                                    \
        (LINKED_LIST) &(pThread)->activeListStruct)



/* when a thread acquires a monitor, it will have an associated depth if it ended up 
 * on the blocked list by exiting from a wait condition 
 */
#define setMonitorDepth(pMonitor, pThread)                                                      \
    if(pThread->pWaitMonitor != NULL) {                                                         \
        pMonitor->depth = pThread->waitMonitorDepth;                                            \
        pThread->pWaitMonitor = NULL;                                                           \
    } else pMonitor->depth = 1;

#define setWaitMonitor(pThread, pMonitor)                                                       \
    pThread->pWaitMonitor = pMonitor;                                                           \
    pThread->waitMonitorDepth = pMonitor->depth
 
#define addThreadToRunningThreads(pThread)                                                      \
    threadListAddThread(runningThreads + pThread->currentPriority, pThread);                    \
    addToPriorityMap(pThread->currentPriority)

#define removeThreadFromRunningThreads(pThread)                                                 \
    threadListRemoveThread(runningThreads + pThread->currentPriority, pThread);                 \
    removePriority(pThread->currentPriority)

#define removePriority(priority)                                                                \
    if(runningThreads[priority] == NULL) removeFromPriorityMap(priority)

#define moveCurrentThreadFromRunningThreads(ppNewList)                                          \
    ASSERT(getCurrentThread() == runningThreads[getCurrentThread()->currentPriority]->pThread); \
    threadListMoveOne(ppNewList, runningThreads + getCurrentThread()->currentPriority);         \
    removePriority(getCurrentThread()->currentPriority)

#define removeCurrentThreadFromRunningThreads()                                                 \
    ASSERT(getCurrentThread() == runningThreads[getCurrentThread()->currentPriority]->pThread); \
    threadListRemoveOne(runningThreads + getCurrentThread()->currentPriority);                  \
    removePriority(getCurrentThread()->currentPriority)

#define changeRunningThreadPriority(oldPriority, newPriority)                                   \
    threadListMoveOne(runningThreads + newPriority, runningThreads + oldPriority);              \
    removePriority(oldPriority);                                                                \
    addToPriorityMap(newPriority)





/* Callbacks for the timer */


/* callbacks are always called from within a critical region */

static void monitorWaitExpired(void *arg)
{
    THREAD pThread = (THREAD) arg;
    MONITOR pMonitor = pThread->pWaitMonitor;
    
    threadListRemoveThread(&pMonitor->pAsleepList, pThread);
    if(pMonitor->pOwner != NULL) {
        threadListAddThread(&pMonitor->pBlockedList, pThread);
        pThread->activeStatus = THREAD_BLOCKED_ON_MONITOR;
    }
    else {
        pMonitor->pOwner = pThread;
        setMonitorDepth(pMonitor, pThread);
        pThread->activeStatus = THREAD_RUNNING;
        addThreadToRunningThreads(pThread);
        /* any context switching will be done from the ISR */
    } 
}



static void threadSleepExpired(void *arg)
{
    THREAD pThread = (THREAD) arg;
    
    pThread->interruptedFlag = FALSE;
    threadListRemoveThread(&asleepList, pThread);
    pThread->activeStatus = THREAD_RUNNING;
    addThreadToRunningThreads(pThread);
    /* any context switching will be done from the ISR */
}






/* to be called only from within a critical region */

static void releaseBlockedThread(MONITOR pMonitor)
{
    THREAD_LIST pBlockedThreads = threadListRemoveOne(&pMonitor->pBlockedList);
    
    if(!pBlockedThreads) {
        pMonitor->pOwner = NULL;
    }
    else {
        THREAD pBlockedThread = pBlockedThreads->pThread;
        pMonitor->pOwner = pBlockedThread;
        setMonitorDepth(pMonitor, pBlockedThread);
        pBlockedThread->activeStatus = THREAD_RUNNING;
        addThreadToRunningThreads(pBlockedThread);
    }
    return;    
}


/* to be called only from within a critical region */

void monitorEnterBlockedMonitor(MONITOR pMonitor)
{
    /* add myself to the monitor's blocked list */
    moveCurrentThreadFromRunningThreads(&pMonitor->pBlockedList);
    getCurrentThread()->activeStatus = THREAD_BLOCKED_ON_MONITOR;
    generatedContextSwitch();
}

/* to be called only from within a critical region */

void monitorExitContendedMonitor(MONITOR pMonitor)
{
    releaseBlockedThread(pMonitor);
    conditionalContextSwitch();
}

/* to be called only from within a critical region */

void setThreadPriority(THREAD pThread, UINT8 priority)
{
    if(pThread->state == THREAD_ACTIVE && pThread->activeStatus == THREAD_RUNNING) {
        if(pThread == getCurrentThread()) {
            changeRunningThreadPriority(pThread->currentPriority, priority);
            pThread->currentPriority = priority;
        }
        else {
            removeThreadFromRunningThreads(pThread);
            pThread->currentPriority = priority;
            addThreadToRunningThreads(pThread);
        }
        conditionalContextSwitch();
    }
    else {
        pThread->currentPriority = priority;
    }
}





/* Thread scheduling API */



void setPriority(THREAD pThread, UINT8 priority)
{
    enterCritical();
    setThreadPriority(pThread, priority);
    exitCritical();
}

UINT32 getActiveThreadCount()
{   
    return activeThreadCount;
}

/* 
 * You must call this function before calling any other function in the
 * scheduler API.
 * 
 * The thread can be scheduled to begin by acquiring a monitor, and hence
 * going on the monitor's blocked list if the monitor is blocked.
 *
 * A thread scheduled by this function will not begin executing right away
 * even if it is the highest priority thread - this function always returns
 * immediately without any context switching.
 *
 * To switch to a newly scheduled highest priority thread right away, weakYield() should
 * be called right after the call to this function.
 */
void scheduleNewThread(THREAD pNewThread, MONITOR pMonitor) 
{
    enterCritical();

    activeThreadCount++;
    activeThreadListAddThread(pNewThread);
    pNewThread->state = THREAD_ACTIVE;
    if(pMonitor != NULL) { 
        if(pMonitor->pOwner == NULL) {
            pMonitor->pOwner = pNewThread;
            pMonitor->depth = 1;
            pNewThread->activeStatus = THREAD_RUNNING;
            addThreadToRunningThreads(pNewThread);
        }
        else {
            threadListAddThread(&pMonitor->pBlockedList, pNewThread);
            pNewThread->activeStatus = THREAD_BLOCKED_ON_MONITOR;
        }
    }
    else {
        pNewThread->activeStatus = THREAD_RUNNING;
        addThreadToRunningThreads(pNewThread);
    }
    exitCritical();
}

/* note: a timeout of zero or NULL indicates an indefinite wait */
void monitorWait(MONITOR pMonitor, ELAPSED_TIME pTimeOut)
{
    THREAD pThread;

    enterCritical();
    pThread = getCurrentThread();
    moveCurrentThreadFromRunningThreads(&pMonitor->pAsleepList);
    setWaitMonitor(pThread, pMonitor);
    releaseBlockedThread(pMonitor);
    if(pTimeOut != NULL && !(timeIsZero(pTimeOut))) { /* this wait has a timeout */
        initializeTimerCallback(&pThread->queueStruct, pTimeOut, monitorWaitExpired, pThread);
        setTimerCallback(&pThread->queueStruct);
        pThread->activeStatus = THREAD_ASLEEP_TIMED_OUT_ON_MONITOR;
    }
    else {
        pThread->activeStatus = THREAD_ASLEEP_ON_MONITOR;
    }
    generatedContextSwitch();
    exitCritical();
}


void monitorNotify(MONITOR pMonitor)
{
    THREAD pThread;

    enterCritical();
    if(pMonitor->pAsleepList) {
        pThread = pMonitor->pAsleepList->pThread;
        if(pThread != NULL) {
            if(pThread->activeStatus == THREAD_ASLEEP_TIMED_OUT_ON_MONITOR) {
                removeTimerCallback(&pThread->queueStruct);
            }
            threadListMoveOne(&pMonitor->pBlockedList, &pMonitor->pAsleepList);
            pThread->interruptedFlag = FALSE;
        }
        conditionalContextSwitch();
    }
    exitCritical();
}

void monitorNotifyAll(MONITOR pMonitor)
{
    THREAD pThread;
    THREAD_LIST pList;

    enterCritical();
    pList = pMonitor->pAsleepList;
    if(pList) {
        ITERATE_THREAD_LIST(pList)
            pThread = pList->pThread;
            if(pThread != NULL) {
                if(pThread->activeStatus == THREAD_ASLEEP_TIMED_OUT_ON_MONITOR) {
                    removeTimerCallback(&pThread->queueStruct);
                }
                pThread->interruptedFlag = FALSE;
            }
        END_ITERATE_THREAD_LIST(pList)
        threadListMoveAll(&pMonitor->pBlockedList, &pMonitor->pAsleepList);
        conditionalContextSwitch();
    }
    exitCritical();
}

/* causes thread to sleep for the specified timeout period 
 */
void threadSleep(ELAPSED_TIME pTimeOut)
{
    TIMER_QUEUE_ENTRY pQueueEntry= &(getCurrentThread()->queueStruct);

    enterCritical();
    initializeTimerCallback(pQueueEntry, pTimeOut, threadSleepExpired, getCurrentThread());
    setTimerCallback(pQueueEntry);
    getCurrentThread()->activeStatus = THREAD_ASLEEP_TIMED_OUT;
    moveCurrentThreadFromRunningThreads(&asleepList);
    generatedContextSwitch(); 
    exitCritical();
}

/* yields to other threads of higher priority 
 */
void weakYield()
{
    enterCritical();
    conditionalContextSwitch();
    exitCritical();
}

/* yields to other threads of equal or higher priority 
 */
void strongYield()
{
    THREAD pNextThread;

    enterCritical();

    /* move the current thread elsewhere by rotating the running thread list,
     * just as if it had been moved to a wait or sleep list
     */
    pNextThread = rotateRunningThreadList()->pThread;
    if(pNextThread != getCurrentThread()) {
        generatedContextSwitch(); 
    }
    exitCritical();
}


/* make the current thread wait for pThreadToJoin to die before resuming execution */

void threadJoin(THREAD pThreadToJoin)
{
    enterCritical();
    moveCurrentThreadFromRunningThreads(&pThreadToJoin->pJoiningThreads);
    getCurrentThread()->activeStatus = THREAD_BLOCKED_ON_THREAD_DEATH;
    getCurrentThread()->pJoinedThread = pThreadToJoin;
    generatedContextSwitch();
    exitCritical();
}

static void releaseJoinedThreads()
{
    THREAD_LIST pList;

    while(pList = threadListRemoveOne(&getCurrentThread()->pJoiningThreads)) {
        pList->pThread->activeStatus = THREAD_RUNNING;
        pList->pThread->interruptedFlag = FALSE;
        addThreadToRunningThreads(pList->pThread);
    }
}

/* same as removeThread except to be called from within a critical region 
 */
void removeThreadCritical()
{
    activeThreadCount--;
    activeThreadListRemoveThread(getCurrentThread());
    releaseJoinedThreads();
    removeCurrentThreadFromRunningThreads();
    getCurrentThread()->state = THREAD_FINISHED;

    if(getCurrentThread()->cleanUp != NULL) {
        getCurrentThread()->cleanUp(getCurrentThread());
    }

    /* since we have removed the thread from the running set, 
     * we will never return after switching threads 
     */
    generatedContextSwitch();
}

/* Called when the current thread dies, this forces a thread switch,
 * so it should be the last thing you do when a thread is meant to die.
 */
void removeThread()
{
    enterCritical();
    removeThreadCritical();
    exitCritical();
}

/* Will interrupt threads that are waiting or sleeping, causing them to resume execution.
 * Not to be confused with ISR interrupts.
 * Has no effect on threads that are blocked on a monitor or a semaphore by other threads, 
 * these threads must wait their turn to proceed.
 */
void interruptThread(THREAD pThread) 
{   
    enterCritical();
    pThread->interruptedFlag = TRUE;
    if(pThread->activeStatus == THREAD_BLOCKED_ON_THREAD_DEATH) {
        pThread->activeStatus = THREAD_RUNNING;
        threadListRemoveThread(&pThread->pJoinedThread->pJoiningThreads, pThread);
        addThreadToRunningThreads(pThread);
        conditionalContextSwitch();
    }
    else if(pThread->activeStatus == THREAD_ASLEEP_ON_MONITOR || pThread->activeStatus == THREAD_ASLEEP_TIMED_OUT_ON_MONITOR) {
        MONITOR pMonitor;
    
        if(pThread->activeStatus == THREAD_ASLEEP_TIMED_OUT_ON_MONITOR) {
            removeTimerCallback(&pThread->queueStruct);
        }
        pMonitor = pThread->pWaitMonitor;
        threadListRemoveThread(pMonitor->pAsleepList, pThread);
        if(pMonitor->pOwner != NULL) {
            threadListAddThread(&pMonitor->pBlockedList, pThread);
            pThread->activeStatus = THREAD_BLOCKED_ON_MONITOR;
        }
        else {
            pMonitor->pOwner = pThread;
            setMonitorDepth(pMonitor, pThread);
            pThread->activeStatus = THREAD_RUNNING;
            addThreadToRunningThreads(pThread);
            conditionalContextSwitch();
        }
    }
    else if(pThread->activeStatus == THREAD_ASLEEP_TIMED_OUT) {
        threadListRemoveThread(&asleepList, pThread);
        pThread->activeStatus = THREAD_RUNNING;
        addThreadToRunningThreads(pThread);
        conditionalContextSwitch();
    }
    exitCritical();
}



void semaphorePost(SEMAPHORE pSemaphore)
{
    enterCritical();
    if(++pSemaphore->counter <= 0) {
        THREAD pBlockedThread = threadListRemoveOne(&pSemaphore->pBlockedList)->pThread;
        pBlockedThread->activeStatus = THREAD_RUNNING;
        addThreadToRunningThreads(pBlockedThread);
        conditionalContextSwitch();   
    }
    exitCritical();
}

void semaphorePend(SEMAPHORE pSemaphore)
{
    enterCritical();
    if(--pSemaphore->counter < 0) {
        THREAD pThread = getCurrentThread();
        pThread->activeStatus = THREAD_BLOCKED_ON_SEMAPHORE;
        moveCurrentThreadToSemaphoreQueue(pSemaphore);
        generatedContextSwitch();
    }
    exitCritical();
}

#if HIGHEST_PRIORITY_SEMAPHORES /* this function used only by priority semaphores */

/* See doubleListAddOneSorted for the requirements of this comparator. 
 * Returns TRUE if a is higher priority than b, FALSE otherwise 
 */
BOOLEAN threadPriorityComparator(THREAD_LIST a, THREAD_LIST b)
{
    return a->pThread->currentPriority > b->pThread->currentPriority;
}

#endif

#if PREEMPTIVE_THREADS

UINT8 reschedulerEnabledIndicator;

void disableRescheduler()
{
    enterCritical();
    reschedulerEnabledIndicator++;
    exitCritical();
}

void enableRescheduler()
{
    enterCritical();
    reschedulerEnabledIndicator--;
    if(reschedulerEnabledIndicator == 0) {
        conditionalContextSwitch();
    }
    exitCritical();
}


#endif








  

