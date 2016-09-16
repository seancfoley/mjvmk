#ifndef THROW_H
#define THROW_H

#include "elements/base.h"
#include "interpret/javaGlobals.h"



typedef struct throwRegisterStruct {
    FRAME fp;
    INSTRUCTION pc;
} throwRegisterStruct, *RESET_REGISTERS, *CODE_REGISTERS;

/* Note that we pass a pointer to the object to be thrown because it could possibly be
 * changed to an IllegalMonitorStateException
 */
void findNewFrame(OBJECT_INSTANCE *ppThrowableObject, RESET_REGISTERS pRegisters);

#define findNewLocation(ppThrowableObject, currentFP, currentPC)                                \
{                                                                                               \
    throwRegisters.fp = currentFP;                                                              \
    throwRegisters.pc = currentPC;                                                              \
    findNewFrame(ppThrowableObject, &throwRegisters);                                           \
}

#if PREEMPTIVE_THREADS

/* monitor used for stack trace output */
extern MONITOR pStackTraceMonitor;

#define enterStackTraceMonitor() monitorEnter(pStackTraceMonitor)
#define exitStackTraceMonitor() monitorExit(pStackTraceMonitor)

#else

#define enterStackTraceMonitor()
#define exitStackTraceMonitor()

#endif


RETURN_CODE createStackTrace(OBJECT_INSTANCE pThrowable, CODE_REGISTERS pRegisters);
void printStackTrace(OBJECT_INSTANCE pStackTrace);
void printCurrentStackTrace(CODE_REGISTERS pRegisters);

#endif
