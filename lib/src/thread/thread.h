#ifndef THREAD_H
#define THREAD_H

#include "psTime.h"
#include "types/linkedlist.h"
#include "types/config.h"
#include "timer.h"

typedef struct threadListStruct threadListStruct, *THREAD_LIST;
typedef struct threadStruct threadStruct, *THREAD;



/* threads are always added to the end of each list, and removed from the beginning 
 * so they are FIFO
 */
typedef struct monitorStruct {
    UINT32 depth; /* the number of times the owning thread has entered the monitor */
    THREAD pOwner;
    THREAD_LIST pAsleepList;   /* TL2 */
    THREAD_LIST pBlockedList;  /* TL3 */
} monitorStruct, *MONITOR;

/* threads are always added to the end of the list, and removed from the beginning 
 * so they are FIFO
 */
typedef struct semaphoreStruct {
    INT32 counter;
    THREAD_LIST pBlockedList;  /* TL6 */
} semaphoreStruct, *SEMAPHORE;

typedef enum threadState {
    THREAD_NOT_STARTED = 0, /* the initial state must be zero, since the whole struct is zeroed when it is created */
    THREAD_ACTIVE,
    THREAD_FINISHED
} threadState;

typedef enum threadActiveStatus {
    THREAD_ASLEEP_TIMED_OUT,            /* Thread.sleep(n) */
    THREAD_ASLEEP_ON_MONITOR,           /* Object.wait */                                             
    THREAD_ASLEEP_TIMED_OUT_ON_MONITOR, /* Object.wait(n) */
    THREAD_BLOCKED_ON_MONITOR,          /* entry into synchronized block prevented */
    THREAD_BLOCKED_ON_THREAD_DEATH,     /* Thread.join */
    THREAD_BLOCKED_ON_SEMAPHORE,
    THREAD_RUNNING                      /* executing byte code */
} threadActiveStatus;

struct threadListStruct { /* must mirror the structure of a linkedListStruct */
    THREAD pThread;
    THREAD_LIST pNext;
    THREAD_LIST pPrevious;
};

#if USE_TIME_SHARES

typedef enum timeShareEnum {
    TIME_SHARE_LOWEST = 1,      /* will accept 1/6 of all alloted cycles */
    TIME_SHARE_LOW = 2,         /* will accept 1/3 of alloted cycles */
    TIME_SHARE_MID_LOW = 3,     /* will accept 1/2 of alloted cycles */
    TIME_SHARE_REGULAR = 6,     /* will accept each alloted cycle */
    TIME_SHARE_MID_HIGH = 12,   /* will secure 2 cycles for every alloted cycle */
    TIME_SHARE_HIGH = 18,       /* will secure 3 cycles for every alloted cycle */
    TIME_SHARE_HIGHEST = 36     /* will secure 6 cycles for every alloted cycle */
} timeShareEnum, TIME_SHARE;

#endif



/* this structure holds all data pertaining to kernel level threads */
struct threadStruct {

#if PREEMPTIVE_THREADS
    nativeStackParameterStruct savedThreadParameters;
    PS_STACK_ELEMENT pNativeStack;
#endif
    threadState state;
    threadActiveStatus activeStatus;

    /* the kernel thread priority */
    UINT8 currentPriority;
    
    /* used as a placeHolder for returning to a previous priority */
    UINT8 savedPriority;

#if USE_TIME_SHARES
    /* the share of time alloted to the thread when competing with other threads at the same priority */
    TIME_SHARE timeShareAllotment;

    /* keeps track of alloted time shares */
    INT8 timeShareCounter;
#endif
    
    THREAD_LIST pJoiningThreads; /* TL4: threads who have joined me */

    THREAD pJoinedThread; /* the thread we've joined */

    /*
     * A thread is either inactive or active.  
     * 
     * An active thread must be stored in exactly one of 6 places at all times:
     * TL1 in the global sleep list
     * TL2 in a monitor's asleep list
     * TL3 in a monitor's blocked list
     * TL4 in the wait list for a thread death
     * TL5 in the running lists
     * TL6 in a semaphore's blocked list
     * 
     * Byte code is executed only for those threads in L5.
     * 
     * See the monitor structure for TL2 and TL3
     *
     * See the semaphore structure for TL6
     */
    threadListStruct listStruct;  /* for storage in exactly one of TL1 to TL6 */
    
    /* all active threads are found in this list at all times */
    threadListStruct activeListStruct;

    /* for storage of the thread on a timer queue */
    timerQueueEntryStruct queueStruct;

    /* If this thread is waiting on a monitor, the monitor goes here along
     * with the ownership depth when it was released (ie might have entered the monitor twice or more before calling wait())
     */
    UINT32 waitMonitorDepth;
    
    /* If this thread is waiting on a monitor, the monitor this thread is waiting on */
    MONITOR pWaitMonitor;

    /* if this thread has been interrupted by a call to interruptThread then this flag is set */
    BOOLEAN interruptedFlag;                          

    /* monitor for thread kill protection, the thread cannot be killed if it is inside this monitor */
    MONITOR pKillProtectionMonitor;
    
    /* this function is called just before this thread is switched out, if non-null */
    void (*prepareSwitchOut)(THREAD pThisThread);

    /* this function is called just before this thread is switched in, if non-null */
    void (*prepareSwitchIn)(THREAD pThisThread);

    /* this function is called just before this thread dies, if non-null */
    void (*cleanUp)(THREAD pThisThread);

};


#if USE_TIME_SHARES
#define setRegularTimeShare(pThread) {(pThread)->timeShareAllotment = TIME_SHARE_REGULAR; (pThread)->timeShareCounter = TIME_SHARE_REGULAR;}
#else
#define setRegularTimeShare(pThread)
#endif


/* must be called on any newly created threadStruct structures */
#define initializeThreadStruct(pInitThread, switchOutFunc, switchInFunc, cleanUpFunc, priority) \
    (pInitThread)->listStruct.pThread = (pInitThread);                                          \
    (pInitThread)->activeListStruct.pThread = (pInitThread);                                    \
    (pInitThread)->currentPriority = priority;                                                  \
    (pInitThread)->prepareSwitchOut = switchOutFunc;                                            \
    (pInitThread)->prepareSwitchIn = switchInFunc;                                              \
    setRegularTimeShare(pInitThread);                                                           \
    (pInitThread)->cleanUp = cleanUpFunc;

extern THREAD pGlobalCurrentThread;

#define getCurrentThread()              (pGlobalCurrentThread)
#define setCurrentThread(pThread)       (pGlobalCurrentThread = (pThread))

extern THREAD pIdleThread;





#endif