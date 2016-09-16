#ifndef CONFIG_H
#define CONFIG_H


#include "psConfig.h"



#if TESTING

#define ASSERT(x) ((x) || assertFailure(#x, __FILE__, __LINE__, __DATE__, __TIME__))
#define FIRST_ASSERT(x) ASSERT(x),

#else

#define ASSERT(x)
#define FIRST_ASSERT(x) 

#endif


#if !PREEMPTIVE_THREADS

/* critical regions are not necessary if there are no thread interrupts */
#define enterCritical()
#define exitCritical()

#endif

/* at the moment there is no additional non-platform specific initialization required */
#define initializeCPU() psInitializeCPU()

#endif
