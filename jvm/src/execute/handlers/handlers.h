
#ifndef HANDLERS_H
#define HANDLERS_H

#include "elements/base.h"
#include "resolution/createThrowable.h"
#include "frameRegisters.h"



#define START_WIDE_INSTRUCTION_HANDLER(instruction) case instruction: {
#define START_INSTRUCTION_HANDLER(instruction) case instruction: {
#define END_HANDLER goto instructionSwitch; }
#define instructionBreak() goto instructionSwitch;




/* Throwing objects */

/* Note we put the thrown object on the stack before we reset the stack pointer so
 * it is not garbage collected.
 */


#define throwThrowableObject(ppThrowableObject, currentFP, currentPC) {                         \
    findNewLocation(ppThrowableObject, currentFP, currentPC);                                   \
    setFP(throwRegisters.fp);                                                                   \
    setPC(throwRegisters.pc);                                                                   \
    setLP(getLocals(throwRegisters.fp));                                                        \
    setCurrentMethodDef(getFrameMethod(throwRegisters.fp));                                     \
    pFieldValue = getResetSP(throwRegisters.fp,                                                 \
        isSynchronizedMethod(getFrameMethod(throwRegisters.fp))) + 1;                           \
    setStackFieldObject(pFieldValue, *ppThrowableObject);                                       \
    setSP(pFieldValue);                                                                         \
    setCurrentClassDef(getCurrentMethodDef()->base.pOwningClass);                               \
    setCurrentConstantPool(getCurrentClassDef()->loadedHeader.pConstantPool);                   \
}




#define throwError(errorCode)                       {getThrowableErrorObject(getCurrentJavaThread(), errorCode, getPC(), getFP()); throwThrowableObject(&getCurrentJavaThread()->pObjectToThrow, getFP(), getPC());}
#define throwException(exceptionCode)               {getThrowableExceptionObject(getCurrentJavaThread(), exceptionCode, getPC(), getFP()); throwThrowableObject(&getCurrentJavaThread()->pObjectToThrow, getFP(), getPC());}
#define throwErrorAndBreak(returnCode)              {throwError(returnCode); instructionBreak();}
#define throwExceptionAndBreak(exceptionCode)       {throwException(exceptionCode); instructionBreak();}




/*
 * Setting the stack pointer first will aid garbage collection a wee bit.
 */

#define popCurrentFrame() popFrame(getFP(), getLP(), 0)


/* pushing frames */ 

/* note that the check for a synchronized frame is not done here, it must be done later */
#define pushStackFrame(pMethodDef) {                                                            \
    FRAME pNewFrame;                                                                            \
                                                                                                \
    pushFrame(pNewFrame, pMethodDef, getSP(), getFP(), getPC(), getStackLimit(),                \
        throwErrorAndBreak);                                                                    \
    setSP((((JSTACK_FIELD) endOfFrame(pNewFrame)) - 1));                                        \
    setPC(pMethodDef->pCode);                                                                   \
    setFP(pNewFrame);                                                                           \
    setLP(getLocals(pNewFrame));                                                                \
    setCurrentMethodDef(pMethodDef);                                                            \
    setCurrentClassDef(pMethodDef->base.pOwningClass);                                          \
    setCurrentConstantPool(pMethodDef->base.pOwningClass->loadedHeader.pConstantPool);          \
}


#define exitSynchronizedMonitor(pMonitor) {                                                     \
    if(monitorNotOwnedByCurrentJavaThread(pMonitor)) {                                          \
        throwExceptionAndBreak(EXCEPTION_CODE_ILLEGAL_MONITOR_STATE);                           \
    }                                                                                           \
    monitorExit(pMonitor);                                                                      \
}

#define exitOrThrowFromBelow(pMonitor) {                                                        \
    if(monitorNotOwnedByCurrentJavaThread(pMonitor)) {                                          \
        popCurrentFrame();                                                                      \
        throwExceptionAndBreak(EXCEPTION_CODE_ILLEGAL_MONITOR_STATE);                           \
    }                                                                                           \
    monitorExit(pMonitor);                                                                      \
}


/* note that if we need to throw something while exiting the monitor, we need to pop the frame first,
 * otherwise the throwable could be caught in the method which has already terminated.
 *
 * Note that this will also make the stack trace start from the method of the frame below.
 */
#define exitSynchronizedMonitorInFrame(fp) exitOrThrowFromBelow(getMonitorFromFrame(fp))





#if PREEMPTIVE_THREADS

#define monitorEnterConditionalNative(pMonitor) monitorEnter(pMonitor)

#else

/* When java thread switches are performed by java register switches, 
 * a synchronized native invoke instruction must be executed twice if the monitor is
 * blocked the first time the instruction is executed.
 * The second time executing the instruction we will already own the monitor, the previous owner
 * having given us ownership, and we should not enter the monitor again.  
 *
 * This all means that we must know if we are entering the instruction a second time, 
 * hence the need for the blockedOnNative flag.
 */
#define monitorEnterConditionalNative(pMonitorVal) {                                            \
    if(getCurrentJavaThread()->blockedOnNative) {                                               \
        getCurrentJavaThread()->blockedOnNative = FALSE;                                        \
    }                                                                                           \
    else {                                                                                      \
        monitorEnter(pMonitorVal);                                                              \
        if(monitorNotOwnedByCurrentJavaThread(pMonitorVal)) {                                   \
            getCurrentJavaThread()->blockedOnNative = TRUE;                                     \
            generatedRegisterSwitch();                                                          \
            instructionBreak();                                                                 \
        }                                                                                       \
    }                                                                                           \
}                                                                                               \

#endif




/* We must push the internal frame for clinit, 
 * then push the arguments as they appear in the clinitDataStruct structure.
 */
#define initializeClass(pClassInstance) {                                                       \
    pushStackFrame(pInternalClinitMethod);                                                      \
    pushOperandStackObject(NULL);                                                               \
    pushOperandStackObject(getClassObjectFromClassInstance(pClassInstance));                    \
    pushOperandStackInt(CLINIT_INITIAL_STAGE);                                                  \
    instructionBreak();                                                                         \
}


/* for the NEWARRAY instruction */
typedef enum arrayTypeEnum {
    T_BOOLEAN = 4,
    T_CHAR = 5,
    T_FLOAT = 6,
    T_DOUBLE = 7,  
    T_BYTE = 8,
    T_SHORT = 9,  
    T_INT = 10, 
    T_LONG = 11  
} ARRAY_TYPE_ENUM;


#endif


