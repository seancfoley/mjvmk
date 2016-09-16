
#include "log.h"

#if LOGGING

#if PREEMPTIVE_THREADS

monitorStruct loggingMonitor = {0, NULL, NULL, NULL};
MONITOR pLoggingMonitor = &loggingMonitor;

#endif

#endif