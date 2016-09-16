
#include "timer.h"

/* the list of threads currently timed out */
TIMER_QUEUE_ENTRY pTimerList = NULL;
UINT32 timerEntry = 0; /* the timer queue entries will have sequential id's */


/* See doubleListAddOneSorted for the requirements of this comparator.
 * Returns TRUE if a is sooner than b, FALSE otherwise 
 */
BOOLEAN timerComparator(TIMER_QUEUE_ENTRY a, TIMER_QUEUE_ENTRY b)
{
    /* timeCmp(a, b) returns TRUE if a < b, whether a, b are absolute or relative times */
    return timeCmp(&a->absoluteTime, &b->absoluteTime);
}




/* This function is called periodically so that timed-out threads 
 * can return to activity if their time-out period has expired.
 * Returns TRUE if at least one thread removed from the queue.
 *
 * This function must be called from within a critical region.
 */
void updateTimerQueue()
{
    timeValueStruct currentTime;

    getCurrentTime(&currentTime);

    /* if we have entered this function the timer queue is not NULL */
    do {
        if(timeCmp(&currentTime, &pTimerList->absoluteTime)) {
            break;
        }
        else {
            TIMER_QUEUE_ENTRY pOneRemoved = removeFirstTimerCallback();
            pOneRemoved->callbackFunction(pOneRemoved->callbackFunctionArgument);
        }
    } while(pTimerList != NULL);

#if PREEMPTIVE_THREADS
    if(pTimerList != NULL) {
        /* determine the time interval until the next entry expires
         */
        pTimerList->timeInterval = pTimerList->absoluteTime;
        subtractTimes(&pTimerList->timeInterval, &currentTime);

        /* set the timer expiry indicator */
        timerExpiry = tickCounter + getTickCount(getMillis(&pTimerList->timeInterval));
    }
#endif
}


void setTimerCallback(TIMER_QUEUE_ENTRY pTimerQueueEntry)
{
    pTimerQueueEntry->id = timerEntry++;
    addTimerCallback(pTimerQueueEntry);

#if PREEMPTIVE_THREADS
    if(pTimerQueueEntry == pTimerList) { /* first on list */
        timerExpiry = tickCounter + getTickCount(getMillis(&pTimerQueueEntry->timeInterval));
    }

#endif
}



void initializeTimerCallback(TIMER_QUEUE_ENTRY pEntry, ELAPSED_TIME pElapsedTime, void (* f)(void *), void *arg)
{
    timeValueStruct currentTime;
    
    getCurrentTime(&currentTime);
    addTimes(&currentTime, pElapsedTime);
    pEntry->absoluteTime = currentTime; /* ANSI struct assignment */
    pEntry->timeInterval = *pElapsedTime; /* ANSI struct assignment */
    pEntry->callbackFunction = f;
    pEntry->callbackFunctionArgument = arg;
    pEntry->pNext = NULL;
    return;
}


