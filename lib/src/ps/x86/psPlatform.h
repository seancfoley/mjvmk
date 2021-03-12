
#ifndef PSPLATFORM_H
#define PSPLATFORM_H

/* the system time data type */

typedef unsigned __int64 pstime;


/* determines whether the highest priority semaphore is the first one released
 * from the queue or whether the order is FIFO
 */
#define HIGHEST_PRIORITY_SEMAPHORES YES


/* 
 * interrupts and context switching code, to be configured
 * when running the operating system as a kernel with multiple
 * native threads.
 */



/* disable and reenable interrupts */

#define enterCritical() {               \
    __asm {                             \
        __asm pushfd                    \
        __asm cli                       \
    }

#define exitCritical()                  \
    __asm {                             \
        __asm popfd                     \
    }                                   \
}

/*

Critical regions are never nested.  Therefore it is up to the user to
define critical regions as they may occur in their own code:  exiting a 
critical regions can either restore the interrupt state present before 
entering the region, or exiting the region may enable interrupts regardless
of whether they were enabled beforehand.  Either choice is fine.

#define enterCritical() {               \
    __asm {                             \
        __asm cli                       \
    }

#define exitCritical()                  \
    __asm {                             \
        __asm sti                       \
    }                                   \
}

*/

/* an element on the native stack */

typedef UINT32 stackElement, *PS_STACK_ELEMENT;


/* describes a native stack location */

typedef struct nativeStackParameterStruct {
    PS_STACK_ELEMENT esp;
} nativeStackParameterStruct, *PS_STACK_PARAMS;


/* the native stacks before and after context switching */

extern PS_STACK_PARAMS pNextThreadParameters;
extern PS_STACK_PARAMS pCurrentThreadParameters;

/* a planned context switch must vector to the configured interrupt handler */

#define CONTEXT_SWITCH_VECTOR_NUMBER 0x30
#define executeContextSwitch() __asm int CONTEXT_SWITCH_VECTOR_NUMBER

/* a specialized function allows for context switching from within interrupts */

void executeContextSwitchFromISR();

/* initialize the native stack before the first context switch to the thread */

void initializeNativeStackParameters(PS_STACK_PARAMS pThreadParameters, PS_STACK_ELEMENT pNativeStack, UINT32 stackSize, void (*pTask)());


/* initialize the native stack of the current thread before we jump to this new stack */

void intializeCurrentThreadNativeStack(PS_STACK_ELEMENT pNativeStack, UINT32 stackSize, void (*pReturnAddress)());

/* the tick interrupt handler */

void tickISR();

/* The interval between TICKS: 
 * Ticks should occur at 10-100 Hz (or 100 to 10 milliseconds between ticks).
 * It is certainly OK to go outside of that range, depending upon the processor.
 * Round up if not exactly an integer.
 */
#define MILLIS_PER_TICK 56

/* if MILLIS_PER_TICK is not exactly an integer we might wish to change this */
#define psGetCurrentTimeInMillis() (((pstime) tickCounter) * MILLIS_PER_TICK)

/* the number of ticks that must occur for the indicated time interval to expire */
#define getTickCount(millis) ((((UINT32) millis) / MILLIS_PER_TICK) + 1)

/* counts the number of ticks that have occurred */
extern UINT32 tickCounter;

/* the value of the tickCounter when the next timer expiry will occur if there is a timer queue entry */
extern UINT32 timerExpiry;

/* start the tick interrupts - should be done after all other initialization is complete */
void startTick();



/* The number of stack elements for the native stack of new threads.  The optimal size should 
 * be as small as possible to accomodate all possible kernel functionality, and no smaller.  
 *
 */
#define NATIVE_STACK_SIZE 2048 /* in the number of stack elements */

#define IDLE_STACK_SIZE 1024



/* Note: there is not much point in running both threaded and idle garbage collection,
 * because threaded garbage collection will always take precedence in the scheduler and
 * idle gc will be superfluous.
 */

/* run a garbage collection when the system is idle - this is generally wise but it could
 * potentially slow the system down if there are short and/or numerous idle periods 
 */
#define IDLE_GC NO

/* run a thread dedicated to garbage collection
 */
#define THREADED_GC NO

/* delay between garbage collections by the threaded garbage collector - one advantage
 * of this is it allows the idle thread and power conservation to kick in if the gc
 * thread is the only one running.
 */
#define GC_DELAY 56 /* in milliseconds */

/* execution can continue in other threads while garbage collecting 
 */
#define CONCURRENT_GC YES




#endif