#ifndef JAVAREGISTERSWITCH_H
#define JAVAREGISTERSWITCH_H



#include "log/logItem.h"

#if PREEMPTIVE_THREADS /* thread switches occur as context switches entirely within the scheduler */


/* java thread switching is handled by kernel-level context switching, we
 * need not concern ourself with the state of the java registers or the state
 * of monitors when java thread switches occur.
 */

#define incrementSwitchTimeCounter()
#define checkForMonitorBlocked(pMonitor)
#define checkforNativeJavaRegisterSwitch()
#define induceJavaThreadSwitch()
#define requestJavaThreadSwitch()
#define checkForJavaThreadSwitch()
#define generatedRegisterSwitch()


#else /* java thread switches are handled explicitly by java register swapping */

extern UINT32 timeCounter;

#define THREAD_TIME_COUNT 50

extern BOOLEAN signalledJavaRegisterSwitch;
extern BOOLEAN requestedJavaRegisterSwitch;

#if LOCAL_FRAME_REGISTERS

#define switchLocalJavaRegisters(pThread) {                             \
    getCurrentJavaThread()->savedJavaRegisters = localFrameRegisters;   \
    localFrameRegisters = (pThread)->savedJavaRegisters;                \
}

#else

#define switchLocalJavaRegisters(pThread)

#endif

#define checkForInterruptedException() {                                \
    if(currentJavaThreadIsInterrupted()) {                              \
        throwException(EXCEPTION_CODE_INTERRUPTED);                     \
        clearCurrentJavaThreadInterruptedFlag();                        \
    }                                                                   \
}    

/* does the thread switching
 * 
 * note: logging below is permitted because the java threads are 
 * user-level threads and not kernel threads
 */
#define javaRegisterSwitch(pThread) {                                   \
    LOG_LINE(("switching from thread %x to thread %x",                  \
            getCurrentJavaThread(), pThread));                          \
    getCurrentThread()->prepareSwitchOut(getCurrentThread());           \
    switchLocalJavaRegisters(pThread);                                  \
    setCurrentThread((THREAD) (pThread));                               \
    (pThread)->header.prepareSwitchIn((THREAD) (pThread));              \
    checkForInterruptedException();                                     \
}




/* time shares affect the selection of threads running 
 * at the same priority.  A single share is equivalent to a clock tick.
 * Higher priority threads may use more than one share at a time, 
 * while lower priority threads may use only a portion
 * of the shares alloted.  Shares are allotted on a round-robin basis.
 *
 * If time sharing is disabled, all threads
 * consume exactly one clock tick on a round-robin basis. 
 */

#if USE_TIME_SHARES

/* the current share has ended */

#define concludeTimeShare() {                                           \
    JAVA_THREAD pThread;                                                \
    deductTimeShare();                                                  \
    checkTimerQueue();                                                  \
    if(timeShareCounterExpired()) {                                     \
        selectRunningThread();                                          \
        pThread = (JAVA_THREAD) getHighestPriorityThread();             \
        javaRegisterSwitch(pThread);                                    \
    }                                                                   \
    else if((pThread = (JAVA_THREAD) getHighestPriorityThread())        \
        != getCurrentJavaThread()) {                                    \
        javaRegisterSwitch(pThread);                                    \
    }                                                                   \
}


/* the current share must end due to a potentially 
 * required java thread switch 
 */

#define haltTimeShare() {                                               \
    deductTimeShare();                                                  \
    if(timeShareCounterExpired()) {                                     \
        selectRunningThread();                                          \
    }                                                                   \
}

#else

#define concludeTimeShare() {                                           \
    JAVA_THREAD pThread;                                                \
    rotateRunningThreadList();                                          \
    checkTimerQueue();                                                  \
    pThread = (JAVA_THREAD) getHighestPriorityThread();                 \
    if(pThread != getCurrentJavaThread()) {                             \
        javaRegisterSwitch(pThread);                                    \
    }                                                                   \
}

#define haltTimeShare() {                                               \
    rotateRunningThreadList();                                          \
}

#endif



/* handles clock ticks */

#define incrementSwitchTimeCounter() {                                  \
    if(timeCounter-- == 0) {                                            \
        scheduledRegisterSwitch();                                      \
        timeCounter = THREAD_TIME_COUNT;                                \
    }                                                                   \
}

/* handles scheduled thread switches */

#define scheduledRegisterSwitch() {                                     \
    concludeTimeShare();                                                \
}

/* handles generated thread switches */

#define generatedRegisterSwitch() {                                     \
    JAVA_THREAD pHighThread;                                            \
    resetTimeShare();                                                   \
    pHighThread = (JAVA_THREAD) getHighestPriorityThread();             \
    javaRegisterSwitch(pHighThread);                                    \
}



/* check if a wait, join, or sleep has been executed, 
 * or if something like a notify has enabled a higher priority thread 
 */

#define checkforNativeJavaRegisterSwitch() {                            \
    if(signalledJavaRegisterSwitch) {       /* wait, sleep, join */     \
        signalledJavaRegisterSwitch = FALSE;                            \
        generatedRegisterSwitch();                                      \
        instructionBreak();                                             \
    }                                                                   \
    else if(requestedJavaRegisterSwitch) {  /* notify */                \
        JAVA_THREAD pThread;                                            \
        requestedJavaRegisterSwitch = FALSE;                            \
        pThread = (JAVA_THREAD) getHighestPriorityThread();             \
        if(pThread != getCurrentJavaThread()) {                         \
            haltTimeShare();                                            \
            javaRegisterSwitch(pThread);                                \
        }                                                               \
    }                                                                   \
}

/* the current thread failed to enter a monitor 
 * and has been moved to the monitor blocked list 
 */

#define checkForMonitorBlocked(pMonitor) {                              \
    if(monitorNotOwnedByCurrentJavaThread(pMonitor)) {                  \
        generatedRegisterSwitch();                                      \
        instructionBreak();                                             \
    }                                                                   \
}

/* we know some other java thread should be switched in: wait, sleep, join */

#define induceJavaThreadSwitch() (signalledJavaRegisterSwitch = TRUE)

/* we request that a switch occur if a higher priority thread is ready: notify */

#define checkForJavaThreadSwitch() {                                    \
    if(getCurrentThread() != getHighestPriorityThread()) {              \
        requestedJavaRegisterSwitch = TRUE;                             \
    }                                                                   \
}


#endif



#endif
