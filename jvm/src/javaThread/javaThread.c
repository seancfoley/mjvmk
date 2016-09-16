
#include "thread/schedule.h"
#include "javaThread.h"

static UINT32 activeJavaThreadCount;
static THREAD_LIST pActiveJavaThreadList;       /* all active threads are in this list */

#define activeJavaThreadListRemoveThread(pThread)                                               \
    doubleListRemoveOne((LINKED_LIST *) &pActiveJavaThreadList,                                 \
        (LINKED_LIST) &(pThread)->activeJavaListStruct)

#define activeJavaThreadListAddThread(pThread)                                                  \
    doubleListAddOneToEnd((LINKED_LIST *) &pActiveJavaThreadList,                               \
        (LINKED_LIST) &(pThread)->activeJavaListStruct)

#define javaThreadListAddThread(ppList, pThread)                                                \
    doubleListAddOneToEnd((LINKED_LIST *) (ppList),                                             \
        (LINKED_LIST) &(pThread)->gcListStruct)


UINT32 getActiveJavaThreadCount()
{   
    return activeJavaThreadCount;
}

/* only to be called by the GC, each time it is called it wipes out the previously created list */
THREAD_LIST gcGetActiveThreadList()
{
    static THREAD_LIST pNewJavaThreadList;
    THREAD_LIST pJavaThreadList;
    JAVA_THREAD pThread;
    
    pNewJavaThreadList = NULL;

    enterCritical();

    pJavaThreadList = pActiveJavaThreadList;
    ITERATE_THREAD_LIST(pJavaThreadList)
        pThread = (JAVA_THREAD) pJavaThreadList->pThread;
        pThread->gcListStruct.pNext = NULL;
        pThread->gcListStruct.pPrevious = NULL;
        javaThreadListAddThread(&pNewJavaThreadList, pThread);
    END_ITERATE_THREAD_LIST(pJavaThreadList)
    
    exitCritical();

    return pNewJavaThreadList;
}

void scheduleNewJavaThread(JAVA_THREAD pNewThread, MONITOR pMonitor)
{
    enterCritical();
    activeJavaThreadCount++;
    activeJavaThreadListAddThread(pNewThread);
    exitCritical();
    scheduleNewThread((THREAD) pNewThread, pMonitor);
}

void removeJavaThread()
{
    enterCritical();
    activeJavaThreadCount--;
    activeJavaThreadListRemoveThread(getCurrentJavaThread());
    /* we must ensure that the thread struct is not garbage collected now
     * that it is no longer in the active thread list so we must
     * call removeThread inside this critical region.
     *
     * We cannot call removeThreadCritical() first because in the preemptive
     * VM it will actually never return.
     */
    removeThreadCritical();
    exitCritical();
}

/* Java thread priorities are mapped to time shares.  
 * See psParameters.h for a description of time shares.
 */

void setJavaPriority(JAVA_THREAD pThread, javaPriorityEnum priority)
{
    pThread->javaPriority = priority;
#if USE_TIME_SHARES
    switch(priority) {
        case JAVA_PRIORITY_LOWEST:
            pThread->header.timeShareAllotment = TIME_SHARE_LOWEST;
            break;
        case JAVA_PRIORITY_LOW:
            pThread->header.timeShareAllotment = TIME_SHARE_LOW;
            break;
        case JAVA_PRIORITY_MID_LOW:
            pThread->header.timeShareAllotment = TIME_SHARE_MID_LOW;
            break;
        case JAVA_PRIORITY_NORMAL:
            pThread->header.timeShareAllotment = TIME_SHARE_REGULAR;
            break;
        case JAVA_PRIORITY_MID_HIGH:
            pThread->header.timeShareAllotment = TIME_SHARE_MID_HIGH;
            break;
        case JAVA_PRIORITY_HIGH:
            pThread->header.timeShareAllotment = TIME_SHARE_HIGH;
            break;
        case JAVA_PRIORITY_HIGHEST:
            pThread->header.timeShareAllotment = TIME_SHARE_HIGHEST;
            break;
    }
#endif
}

javaPriorityEnum getJavaPriority(JAVA_THREAD pThread)
{
    return pThread->javaPriority;
}


void disableNonCurrentJavaThreads()
{
    THREAD_LIST pList;
    THREAD pThread;

    /* the critical region is required so that no thread is added or removed while we
     * iterate through the list.
     */
    enterCritical();
    pList = pActiveJavaThreadList;
    ITERATE_THREAD_LIST(pList)
        pThread = pList->pThread;
        if(pThread != getCurrentThread()) {
            pThread->savedPriority = getThreadPriority(pThread);
            /* 
             * threads are disabled by not literally disabling them, but instead
             * by setting their priority lower than that of the idle thread 
             */
            setThreadPriority(pThread, THREAD_INACTIVE_PRIORITY);
        }
    END_ITERATE_THREAD_LIST(pList)
    exitCritical();
}

void reenableNonCurrentJavaThreads()
{
    THREAD_LIST pList;
    THREAD pThread;
    
    /* the critical region is needed because as we reenable threads
     * we do not want the list to change.
     */
	enterCritical();
    pList = pActiveJavaThreadList;
    ITERATE_THREAD_LIST(pList)
        pThread = pList->pThread;
        if(pThread != getCurrentThread() && 
            getThreadPriority(pThread) == THREAD_INACTIVE_PRIORITY) {
                setThreadPriority(pThread, pThread->savedPriority);
        }
    END_ITERATE_THREAD_LIST(pList)
    exitCritical();
}

