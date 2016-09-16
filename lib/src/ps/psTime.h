#ifndef PSTIME_H
#define PSTIME_H

#include "psTypes.h"
#include "psPlatform.h"




/* The following struct represents both absolute times and relative times.
 * It allows you to determine the data types and precision involved in time keeping 
 * for the java virtual machine, which may or may not coincide with the same precision
 * provided by the system time.
 */

/* the internals of this struct are not known outside of this file.  
 * The macros below are the only means by which the structure is accessed.
 * Allows for configurable granularity.
 */
typedef struct timeValueStruct {
    pstime milliseconds;          
} timeValueStruct, *ABSOLUTE_TIME, *ELAPSED_TIME, *TIME_VALUE;



/* get the current time in milliseconds since January 1, 1970 00:00:00.000 UTC */
#define getCurrentTime(pTime)                           (((TIME_VALUE) (pTime))->milliseconds = psGetCurrentTimeInMillis())

/* both of the following return TRUE if a < b */
#define timeCmpMillis(pTime1, millis)                   (((TIME_VALUE) (pTime1))->milliseconds < (millis))
#define timeCmp(pTime1, pTime2)                         timeCmpMillis(pTime1, ((TIME_VALUE) (pTime2))->milliseconds)

/* the given elapsed time is zero */
#define timeIsZero(pTime)                               (((TIME_VALUE) pTime)->milliseconds == 0)

/* adds the elapsed time pTime2 to the absolute or elapsed time pTime1 */
#define addTimes(pTime1, pTime2)                        (((TIME_VALUE) (pTime1))->milliseconds += ((TIME_VALUE) (pTime2))->milliseconds)

/* adds the absolute or elapsed time pTime2 from the absolute or elapsed time pTime1 */
#define subtractTimes(pTime1, pTime2)                   (((TIME_VALUE) (pTime1))->milliseconds -= ((TIME_VALUE) (pTime2))->milliseconds)

/* subtracts the given number of milliseconds from the given time value */
#define subtractMillis(pTime, millis)                   (((TIME_VALUE) (pTime))->milliseconds -= (millis))

/* adds the given number of milliseconds to the given time value */
#define addMillis(pTime, millis)                        (((TIME_VALUE) (pTime))->milliseconds += (millis))

/* sets the time value to the given number of milliseconds*/
#define setMillis(pTime, millis)                        (((TIME_VALUE) (pTime))->milliseconds = (millis))

/* the time value rounded up to the number of milliseconds */
#define getMillis(pTime)                                (((TIME_VALUE) (pTime))->milliseconds)

#endif

