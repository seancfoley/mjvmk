
#include "psTime.h"
#include "psIo.h"
#include "psConfig.h"



/* 
 * Reads a byte from the specified port.
 *
 * The microsoft compiler uses eax to return values, a 32-bit register,
 * which contains the 8-bit register al, so we put the byte to be returned in al.
 */
UINT8 portReadByte(UINT16 port)
{
    UINT8 status;

    __asm {
        push edx
        push eax
        mov dx, port
        in al, dx
        mov status, al
        pop eax
        pop edx
    }

    return status;
}

/* 
 * Writes a byte to a specified port.
 */
void portWriteByte(UINT16 port, UINT8 value)
{
    __asm {
        push edx
        push eax
        mov dx, port
        mov al, value
        out dx, al
        pop eax
        pop edx
    }
}


void intializeProgrammableInterruptController()
{
    /* Reprogram the master and slave PIC */

/* Intel 8259 programmable interrupt controller ports */

#define PIC1_PORT1 0x020 /* port 1 of master PIC */
#define PIC1_PORT2 0x021 /* port 2 of master PIC */
#define PIC2_PORT1 0x0a0 /* port 1 of slave PIC */
#define PIC2_PORT2 0x0a1 /* port 2 of slave PIC */

    /* Relocate IRQs to 0x20-0x2F */

    portWriteByte(PIC1_PORT1, 0x11);
	portWriteByte(PIC1_PORT2, 0x20);
	portWriteByte(PIC1_PORT2, 0x04);
	portWriteByte(PIC1_PORT2, 0x01);
	portWriteByte(PIC1_PORT2, 0x00);

	portWriteByte(PIC2_PORT1, 0x11);
	portWriteByte(PIC2_PORT2, 0x28);
	portWriteByte(PIC2_PORT2, 0x02);
	portWriteByte(PIC2_PORT2, 0x01);
	portWriteByte(PIC2_PORT2, 0x00);
}




#define IDT_ENTRY_TYPE_INTERRUPT 0x8e00
#define IDT_ENTRY_TYPE_TRAP 0x8f00
#define IDT_ENTRY_TYPE_TASK 0x8500

#pragma pack(push, 1) /* store members in the following structure on 1-byte boundaries */

typedef struct interruptDispatchTableEntry {
    UINT16 addressLowWord;
    UINT16 segment;
    UINT16 type;
    UINT16 addressHighWord;
} interruptDispatchTableEntry, *INTERRUPT_DISPATCH;

#pragma pack(pop) /* restore whatever pack settings the compiler had previous to the push */



extern INTERRUPT_DISPATCH Idt;
    
void setInterruptDispatchTableEntry(int interruptNumber, UINT16 segment, UINT32 address)
{
    INTERRUPT_DISPATCH pDispatch;

    Idt = (INTERRUPT_DISPATCH) 0x1070;
    pDispatch = Idt + interruptNumber;
    pDispatch->addressHighWord = (UINT16) (address >> 16);
    pDispatch->addressLowWord = (UINT16) (address & 0xffff);
    pDispatch->segment = segment;
    pDispatch->type = IDT_ENTRY_TYPE_INTERRUPT;

}



/* Because the 8086 and 8088 chips have a 20-bit address spaces, their
 * highest addressable memory location is one byte below 1MB.  If you specify
 * an address at 1MB or over, which would require a twenty-first bit to set,
 * the address wraps back to zero.  Some parts of DOS depend on this wrap, so
 * on the 286 and 386, the twenty-first address bit is disabled.  To address
 * extended memory, DOS/4GW enables the twenty-first address bit (the A20
 * line).  The A20 line must be enabled for the CPU to run in protected mode,
 * but it may be either enabled or disabled in real mode.
 *
 * Here we must enable the address line 20 to enter protected mode.
 * This is done by sending commands to the keyboard controller (i8042).
 * Before sending commands, the controller must be polled to ensure that
 * it can receive data.
 */

void enableAddressLine20()
{
    /* Wait until i8042 can receive the command */
	while (portReadByte(0x64) & 0x02);

    /* Send the 'write' command to the keyboard controller */
	portWriteByte(0x64, 0xd1);

    /* Wait again */
	while (portReadByte(0x64) & 0x02);

    /* enable the 20th bit of memory addresses */
    portWriteByte(0x60, 0xdf);

    /* Wait again */
	while (portReadByte(0x64) & 0x02);
}


PS_STACK_PARAMS pCurrentThreadParameters;
PS_STACK_PARAMS pNextThreadParameters;



/* this function catches any threads that actually hit the bottom of their stack, which should not ever happen */

void baseFunction() 
{
    psPrintErr("Error: Hit bottom");
    while(TRUE);
}

#define CONFIGURED_CS 0x08

void intializeCurrentThreadNativeStack(PS_STACK_ELEMENT pNativeStack, UINT32 stackSize, void (*pReturnAddress)())
{
    PS_STACK_ELEMENT pStack;
    
    pStack = pNativeStack + stackSize - 1;

    /* if we should ever return from pReturnAddress then we will land in baseFunction */

    *pStack = 0x00000002; /* flags, see below */
    *--pStack = CONFIGURED_CS;
    *--pStack = (stackElement) baseFunction;
    

    /* call stack frame for pReturnAddress */

    *--pStack = 0x00000002; /* flags, will be popped into EFLAGS reg, interrupts enabled:0x00000202, interrupts disabled:0x00000002 */
    *--pStack = CONFIGURED_CS;
    *--pStack = (stackElement) pReturnAddress;

    
    __asm {
        mov esp, pStack
        ret /* if we do not return here then the compiled return will 
             * attempt to pop the local pStack off the stack,
             * which we have not made room for since we no longer need it.
             * So we must return without popping anything.
             */
    }
}

/* 
 * Initializes the native stack of a new task and stores the stack pointer in pThreadParameters
 */
void initializeNativeStackParameters(PS_STACK_PARAMS pThreadParameters, PS_STACK_ELEMENT pNativeStack, UINT32 stackSize, void (*pTask)())
{
    PS_STACK_ELEMENT pStack;
    PS_STACK_ELEMENT pBaseStackPointer;

    /* first, we must simulate the top of the stack when the 'task' function would be called.
     * next, we must simulate the top of the stack when an interrupt occurs.
     * thirdly, we must duplicate the register pushing that occurs in the above context switches 
     */
    
    pStack = pNativeStack + stackSize - 1;
    
    /* call stack frame for baseFunction */
    
    *pStack = 0x00000202; /* flags, see below */
    *--pStack = CONFIGURED_CS;
    *--pStack = (stackElement) baseFunction;
    pBaseStackPointer = pStack;
    
    /* call stack frame for pReturnAddress */
    *--pStack = 0x00000202; /* flags, will be popped into EFLAGS reg, interrupts enabled:0x00000202, interrupts disabled:0x00000002 */
    *--pStack = CONFIGURED_CS;
    *--pStack = (stackElement) pTask;
    *--pStack = 0; /* eax */
	*--pStack = 0; /* ecx */
	*--pStack = 0; /* edx */
	*--pStack = 0; /* ebx */
	*--pStack = (stackElement) pBaseStackPointer; /* esp */ 
                /* the intel spec states that the popad instruction discards the pushed esp,
                 * so setting this value is unnecessary, 
                 * but for clarity I've set it here to the value that esp will take on
                 */
	*--pStack = 0; /* ebp */
	*--pStack = 0; /* esi */
	*--pStack = 0; /* edi */
    pThreadParameters->esp = pStack;
}

void psSleep()
{
    return;
}

SUCCESS_CODE psInitializeHeap(void **pHeap, size_t heapSize)
{
    *pHeap = (void *) 0xb00000;
    return SC_SUCCESS;
}

extern void simpleISR();
extern void contextSwitchISR();

SUCCESS_CODE psInitializeCPU()
{
    int	interruptNumber;

    enableAddressLine20();
	
    intializeProgrammableInterruptController();

    /* Install a default handler for all supported interrupts:
	 * 0x00-0x1F: reserved by intel
	 * 0x20-0x2F: relocated IRQ's 
	 * 0x30-0x3F: available to the kernel
     */
    for(interruptNumber=0; interruptNumber < 0x40; interruptNumber++) {
        setInterruptDispatchTableEntry(interruptNumber, CONFIGURED_CS, (UINT32) simpleISR);
    }

    /* install the context switch ISR */
	setInterruptDispatchTableEntry(CONTEXT_SWITCH_VECTOR_NUMBER, CONFIGURED_CS, (UINT32) contextSwitchISR);			

    return SC_SUCCESS;
}

void setTickISR(void (*isr)())
{
    setInterruptDispatchTableEntry(0x20, CONFIGURED_CS, (UINT32) isr);
    return;
}   


void assertFailure(char *str, char *file, int line, char *compilationDate, char *compilationTime) 
{
    return;
}










