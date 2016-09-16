#ifndef OPERATE_H
#define OPERATE_H

#include "psTime.h"
#include "thread/thread.h"

#if PREEMPTIVE_THREADS

void idleFunction();

void garbageCollectionFunction();

SUCCESS_CODE initializeNativeStack(PS_STACK_PARAMS pThreadParameters, void (*pTask)(), PS_STACK_ELEMENT *ppStack);

SUCCESS_CODE initializeIdleStack(void (*pReturnAddress)());

void cleanUpNativeStack(THREAD pThread);

SUCCESS_CODE startNativeThread(void (*pTask)(), void (*pSwitchOut)(THREAD), void (*pSwitchIn)(THREAD), MONITOR pMonitor, UINT8 priority, THREAD *ppThread);

SUCCESS_CODE becomeIdleThread(void (*pReturnAddress)());

#if THREADED_GC
extern THREAD pGCThread;
#endif

#endif

SUCCESS_CODE initializeOS();


#endif