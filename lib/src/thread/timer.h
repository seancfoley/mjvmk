#ifndef TIMER_H
#define TIMER_H

#include "types/linkedlist.h"
#include "psTime.h"


typedef struct timerQueueEntryStruct timerQueueEntryStruct, *TIMER_QUEUE_ENTRY;

/* must have the pre-defined requirements of a linked list structure */
struct timerQueueEntryStruct {
    UINT32 id;
    TIMER_QUEUE_ENTRY pNext;
    TIMER_QUEUE_ENTRY pPrevious;
    void (*callbackFunction)(void *);
    void *callbackFunctionArgument;
    timeValueStruct timeInterval; /* the interval before the time-out should occur */
    timeValueStruct absoluteTime; /* when in absolute time the time-out should occur */
};

BOOLEAN timerComparator(TIMER_QUEUE_ENTRY a, TIMER_QUEUE_ENTRY b);
void initializeTimerCallback(TIMER_QUEUE_ENTRY pEntry, ELAPSED_TIME pElapsedTime, void (* f)(void *), void *arg);
void updateTimerQueue();

extern TIMER_QUEUE_ENTRY pTimerList;


#define addTimerCallback(pTimerQueueEntry)                          \
    doubleListAddOneSorted((LINKED_LIST *) &pTimerList,             \
        (LINKED_LIST) pTimerQueueEntry,                             \
        (BOOLEAN (* )(LINKED_LIST, LINKED_LIST)) timerComparator)



/* timer API - all functions and macros must be called from within critical regions */

#if PREEMPTIVE_THREADS

/* the tickCounter is used to avoid unnecessary function calls when there is an entry on the queue */
#define checkTimerQueue() {                                         \
    if(pTimerList != NULL && tickCounter >= timerExpiry)            \
        updateTimerQueue();                                         \
}

#else

#define checkTimerQueue() {                                         \
    if(pTimerList != NULL)                                          \
        updateTimerQueue();                                         \
}

#endif

void setTimerCallback(TIMER_QUEUE_ENTRY pTimerQueueEntry);

#define removeTimerCallback(pTimerQueueEntry)                       \
    doubleListRemoveOne((LINKED_LIST *) &pTimerList, (LINKED_LIST) pTimerQueueEntry)

#define removeFirstTimerCallback()                                  \
    ((TIMER_QUEUE_ENTRY) doubleListRemoveOneFromFront((LINKED_LIST *) &pTimerList))

#endif
