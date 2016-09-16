
#include "thread/thread.h"
#include "contextSwitch.h"

#if PREEMPTIVE_THREADS


/* note: interrupts are always disabled when these functions are called */


void switchContext(THREAD pCurrentThread, THREAD pNextThread)
{
    if(pCurrentThread->prepareSwitchOut) {
        pCurrentThread->prepareSwitchOut(pCurrentThread);
    }
    
    pCurrentThreadParameters = &pCurrentThread->savedThreadParameters;
    pNextThreadParameters = &pNextThread->savedThreadParameters;
    setCurrentThread(pNextThread);
    if(pNextThread->prepareSwitchIn) {
        pNextThread->prepareSwitchIn(pNextThread);
    }
    executeContextSwitch(); /* switch the current thread out  */

    /* we will reach here only when we have been switched back in */
    return;
}


void switchContextFromInterrupt(THREAD pCurrentThread, THREAD pNextThread)
{
    if(pCurrentThread->prepareSwitchOut) {
        pCurrentThread->prepareSwitchOut(pCurrentThread);
    }
    pCurrentThreadParameters = &pCurrentThread->savedThreadParameters;
    pNextThreadParameters = &pNextThread->savedThreadParameters;

    setCurrentThread(pNextThread);
    if(pNextThread->prepareSwitchIn) {
        pNextThread->prepareSwitchIn(pNextThread);
    }
    executeContextSwitchFromISR(); /* switch the current thread out  */

    /* we will never reach here */
    return;
}

#endif
