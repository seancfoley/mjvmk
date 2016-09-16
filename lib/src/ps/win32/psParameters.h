
#ifndef PSPARAMETERS_H
#define PSPARAMETERS_H

#define YES 1 /* do not change */
#define NO 0 /* do not change */

#define TESTING NO

#define LOGGING NO


/* Defines whether the system will have multiple kernel threads and will perform context switching.
 * If set to NO, then the system will have a single kernel thread, although the scheduler will still 
 * do the book-keeping required to run multiple user-level threads.
 */
#define PREEMPTIVE_THREADS NO

/* Time shares determine the share of CPU time a thread is
 * alloted when competing with other threads at the same kernel priority.  
 * A lower share value will cause a thread to voluntarily decline alloted CPU time, 
 * while a higher value will cause a thread to secure extra CPU time.
 */
#define USE_TIME_SHARES YES

/* size of the heap in bytes */
#define HEAP_SIZE (1000 * 1024) /* 1 MB */

#define USE_FILE_SYSTEM YES



#endif