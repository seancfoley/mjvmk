
#ifndef CONTEXTSWITCH_H
#define CONTEXTSWITCH_H

#include "schedule.h"


#if PREEMPTIVE_THREADS

/* all macros must be called from within a critical region */

void switchContext(THREAD pCurrentThread, THREAD pNextThread);
void switchContextFromInterrupt(THREAD pCurrentThread, THREAD pNextThread);



#if USE_TIME_SHARES

/* this macro to be called by the tick interrupt */

/* the current share has ended on its own 
 * we check for a thread switch if the time shares of the thread have expired, or if
 * a timer queue entry has expired
 */
#define concludeTimeShare() {                                           \
    THREAD pCurThread, pHighestThread;                                  \
    checkTimerQueue();                                                  \
    deductTimeShare();                                                  \
    if(timeShareCounterExpired()) {                                     \
        selectRunningThread();                                          \
    }                                                                   \
    if((pHighestThread = getHighestPriorityThread()) !=                 \
        (pCurThread = getCurrentThread())                               \
        switchContextFromInterrupt(pCurThread, pHighestThread);         \
    }                                                                   \
}


/* the current share must end due to a potentially 
 * required java thread switch, so regardless of whether
 * the time shares of this thread are expired we switch threads
 */
#define haltTimeShare() {                                               \
    deductTimeShare();                                                  \
    if(timeShareCounterExpired()) {                                     \
        selectRunningThread();                                          \
    }                                                                   \
}

#else

/* this macro to be called by the tick interrupt */

#define concludeTimeShare() {                                           \
    THREAD pCurThread, pHighestThread;                                  \
    checkTimerQueue();                                                  \
    rotateRunningThreadList();                                          \
    if((pHighestThread = getHighestPriorityThread()) !=                 \
        (pCurThread = getCurrentThread())) {                            \
        switchContextFromInterrupt(pCurThread, pHighestThread);         \
    }                                                                   \
}


#define haltTimeShare() {                                               \
    rotateRunningThreadList();                                          \
}


#endif


/* The following 4 macros are for the four different types of context (thread) switching 
 * that may occur.  
 * 
 * The first is for interrupts that may wish to switch context when
 * the ISR completes.
 * 
 * The second is used when a higher priority thread might have become active.
 *
 * The third is used when the current thread is no longer active.
 *
 * The fourth is for scheduled context switching in accordance with clock ticks,
 * doing a round-robin amongst all threads of the highest priority.
 */

/* This macro to be called by all driver interrupts which might require a thread switch.
 * Will cause an immediate switch to the highest priority thread if there are threads
 * with higher priority than the current thread ready to run.
 *
 * You must ensure that executeContextSwitchFromISR() will function for the ISR in question, 
 * which is a platform and ISR specific issue.
 */
#define interruptConditionalContextSwitch() {                           \
    THREAD pCurThread, pHighestThread;                                  \
    checkTimerQueue();                                                  \
    if((pHighestThread = getHighestPriorityThread()) !=                 \
        (pCurThread = getCurrentThread())) {                            \
        haltTimeShare();                                                \
        switchContextFromInterrupt(pCurThread, pHighestThread);         \
    }                                                                   \
}


/* switch threads if there is a higher priority thread ready
 * or another thread at the same priority ready
 */
#define conditionalContextSwitch() {                                    \
    THREAD pCurThread, pHighestThread;                                  \
    if((pHighestThread = getHighestPriorityThread()) !=                 \
        (pCurThread = getCurrentThread())) {                            \
        haltTimeShare();                                                \
        switchContext(pCurThread, pHighestThread);                      \
    }                                                                   \
}


/* handles generated thread switches */

#define generatedContextSwitch() {                                      \
    resetTimeShare();                                                   \
    switchContext(getCurrentThread(), getHighestPriorityThread());      \
}

/* handles scheduled contextSwitches */

#define scheduledContextSwitch() {                                      \
    concludeTimeShare();                                                \
}

    
#else

#define generatedContextSwitch()
#define conditionalContextSwitch()

#endif



#endif