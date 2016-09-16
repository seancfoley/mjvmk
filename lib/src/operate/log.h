#ifndef LOG_H
#define LOG_H

#include "psIo.h"
#include "thread/schedule.h"


#if LOGGING


#if PREEMPTIVE_THREADS

extern MONITOR pLoggingMonitor;

#define enterLoggingMonitor() monitorEnter(pLoggingMonitor)
#define exitLoggingMonitor() monitorExit(pLoggingMonitor)

#else

#define enterLoggingMonitor()
#define exitLoggingMonitor()

#endif

#define LOG_NEW_LINE() logNewLine()
#define LOG_FLUSH() logFlush()
#define LOG_LINE(x) {enterLoggingMonitor(); logPrintf x; logNewLine(); exitLoggingMonitor();}
#define LOG_LOCATION() LOG_LINE(("%s line %d", __FILE__, __LINE__))

#else

#define LOG_NEW_LINE()
#define LOG_FLUSH()
#define LOG_LINE(x)
#define LOG_LOCATION()

#endif




#endif