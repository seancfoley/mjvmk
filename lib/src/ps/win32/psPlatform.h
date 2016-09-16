
#ifndef PSPLATFORM_H
#define PSPLATFORM_H

/* the system time data type */

typedef unsigned __int64 pstime;


pstime psGetCurrentTimeInMillis();

/* determines whether the highest priority semaphore is the first one released
 * from the queue or whether the order is FIFO
 */
#define HIGHEST_PRIORITY_SEMAPHORES YES

#endif