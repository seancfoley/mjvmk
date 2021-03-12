
#include "thread/contextSwitch.h"
#include "psConfig.h"
#include "psTime.h"

/* number of tick interrupts that occur before a thread reschedule */
#define TICKS_PER_RESCHEDULE 8

UINT32 tickCounter = 0;
UINT32 timerExpiry;

void handleTick()
{
    tickCounter++;
    if(reschedulerEnabledIndicator == 0) {
        if((tickCounter % TICKS_PER_RESCHEDULE) == 0) {
            enterCritical();
            scheduledContextSwitch();
            exitCritical(); /* we might never reach this line because of a context switch,
                             * even when this thread is switched back in, because its saved stack will be 
                             * altered during the switch.
                             */
        }
        else {
            /* we will not perform a rotation among equal priority threads, 
             * but we do check for higher priority threads.
             *
             * If TICKS_PER_RESCHEDULE is 1 we will never reach here.               
             */
            enterCritical();
            interruptConditionalContextSwitch();
            exitCritical();
        }
    }
    /*
     * No need to worry about enabling or disabling interrupts, the flags will be
     * restored when the ISR returns.
     */                        
}

/* install the tick ISR while enabling interrupts */
void startTick()
{
    setTickISR(tickISR);
    
    /* now enable the interrupts */
    __asm sti
    return;
}

