
#ifndef NATIVES_H
#define NATIVES_H


#include "interpret/instructions.h"
#include "resolution/createThrowable.h"






#if LOCAL_FRAME_REGISTERS

#define getSPNative()                               (pFrameRegisterAccessor->sp)
#define getFPNative()                               (pFrameRegisterAccessor->fp)
#define getPCNative()                               (pFrameRegisterAccessor->pc)
#define getLPNative()                               (pFrameRegisterAccessor->lp)


#define setSPNative(newsp)                          (pFrameRegisterAccessor->sp = (newsp))
#define setFPNative(newfp)                          (pFrameRegisterAccessor->fp = (newfp))
#define setPCNative(newpc)                          (pFrameRegisterAccessor->pc = (newpc))
#define setLPNative(newlp)                          (pFrameRegisterAccessor->lp = (newlp))


#if USE_CLASS_REGISTERS

#define getCurrentConstantPoolNative()              (pFrameRegisterAccessor->pCurrentConstantPool)
#define getCurrentClassDefNative()                  (pFrameRegisterAccessor->pCurrentClassDef)
#define getCurrentMethodDefNative()                 (pFrameRegisterAccessor->pCurrentMethodDef)

#define setCurrentConstantPoolNative(pConstantPool) (pFrameRegisterAccessor->pCurrentConstantPool = (pConstantPool))
#define setCurrentClassDefNative(pClassDef)         (pFrameRegisterAccessor->pCurrentClassDef = (pClassDef))
#define setCurrentMethodDefNative(pMethodDef)       (pFrameRegisterAccessor->pCurrentMethodDef = (pMethodDef)) 

#else

#define getCurrentMethodDefNative()                 (getFrameMethod(getFPNative()))
#define getCurrentClassDefNative()                  (getCurrentMethodDefNative()->base.pOwningClass)
#define getCurrentConstantPoolNative()              (getCurrentClassDefNative()->loadedHeader.pConstantPool)

#define setCurrentConstantPoolNative(pConstantPool)
#define setCurrentClassDefNative(pClassDef)
#define setCurrentMethodDefNative(pMethodDef)

#endif

#else

#define getSPNative()                               getSP()
#define getFPNative()                               getFP()
#define getPCNative()                               getPC()
#define getLPNative()                               getLP()


#define setSPNative(newsp)                          setSP(newsp)
#define setFPNative(newfp)                          setFP(newfp)
#define setPCNative(newpc)                          setPC(newpc)
#define setLPNative(newlp)                          setLP(newlp)

#define getCurrentMethodDefNative()                 getCurrentMethodDef()
#define getCurrentClassDefNative()                  getCurrentClassDef()
#define getCurrentConstantPoolNative()              getCurrentConstantPool()

#define setCurrentConstantPoolNative(pConstantPool) setCurrentConstantPool(pConstantPool)
#define setCurrentClassDefNative(pClassDef)         setCurrentClassDef(pClassDef)
#define setCurrentMethodDefNative(pMethodDef)       setCurrentMethodDef(pMethodDef)


#endif


#define setRegistersNative(fp, pMethod) {                                                       \
    setSPNative((((JSTACK_FIELD) endOfFrame(fp)) - 1));                                         \
    setPCNative(pMethod->pCode);                                                                \
    setFPNative(fp);                                                                            \
    setLPNative(getLocals(fp));                                                                 \
    setCurrentMethodDefNative(pMethod);                                                         \
    setCurrentClassDefNative(pMethod->base.pOwningClass);                                       \
    setCurrentConstantPoolNative(pMethod->base.pOwningClass->loadedHeader.pConstantPool);       \
}






/* program counter operations */

#define incrementNativeProgramCounterBy(num) (getPCNative()+=(num))

/* The same native function can either result from an invokevirtual or invokeinterface, so
 * we must ensure the right program counter increment is used. 
 */ 
#define incrementNativeProgramCounterVirtual()      \
    switch(*getPCNative()) {                        \
        case INVOKEINTERFACE:                       \
        case INVOKEINTERFACE_QUICK:                 \
            incrementNativeProgramCounterBy(5);     \
            break;                                  \
        default:                                    \
            incrementNativeProgramCounterBy(3);     \
    }

#define incrementNativeProgramCounterRegular() incrementNativeProgramCounterBy(3)



/* operand stack operations */


#define pStackEntryNative(index)                    (getSPNative() - (index))


#define getOperandStackObjectNative(index)          getStackFieldObject(pStackEntryNative(index))
#define getOperandStackObjectPointerNative(index)   getStackFieldObjectPtr(pStackEntryNative(index))
#define getOperandStackIntNative(index)             getStackFieldInt(pStackEntryNative(index))
#define getOperandStackLongNative(index)            getStackFieldLong(pStackEntryNative(index))

#define pushOperandStackObjectNative(x)             setStackFieldObject(++getSPNative(), x)
#define pushOperandStackIntNative(x)                setStackFieldInt(++getSPNative(), x)
#define pushOperandStackLongNative(x)               {++getSPNative(); setStackFieldLong(getSPNative(), x); ++getSPNative();}

#define setOperandStackObjectNative(index, x)       setStackFieldObject(pStackEntryNative(index), x)
#define setOperandStackIntNative(index, x)          setStackFieldInt(pStackEntryNative(index), x)
#define setOperandStackLongNative(index, x)         setStackFieldLong(pStackEntryNative(index), x)

#define decrementOperandStackNative()               (getSPNative()--)
#define decrementOperandStackNativeBy(num)          (getSPNative()-=num)
#define incrementOperandStackNative()               (getSPNative()++)





/* stack limit operations */


#if GROW_STACK
#define getStackLimitNative() (getFrameStackLimit(getFPNative()))
#else
#define getStackLimitNative() (getCurrentJavaThread()->pStackLimit)
#endif



#define throwThrowableObjectFromNative(ppThrowableObject, currentFP, currentPC) {               \
    findNewLocation(ppThrowableObject, currentFP, currentPC);                                   \
    setFPNative(throwRegisters.fp);                                                             \
    setPCNative(throwRegisters.pc);                                                             \
    setLPNative(getLocals(throwRegisters.fp));                                                  \
    setCurrentMethodDefNative(getFrameMethod(throwRegisters.fp));                               \
    pFieldValue = getResetSP(throwRegisters.fp,                                                 \
        isSynchronizedMethod(getFrameMethod(throwRegisters.fp))) + 1;                           \
    setStackFieldObject(pFieldValue, *ppThrowableObject);                                       \
    setSPNative(pFieldValue);                                                                   \
    setCurrentClassDefNative(getCurrentMethodDefNative()->base.pOwningClass);                   \
    setCurrentConstantPoolNative(getCurrentClassDefNative()->loadedHeader.pConstantPool);       \
}

#define throwErrorFromNative(errorCode)                     {getThrowableErrorObject(getCurrentJavaThread(), errorCode, getPCNative(), getFPNative()); throwThrowableObjectFromNative(&getCurrentJavaThread()->pObjectToThrow, getFPNative(), getPCNative());}
#define throwExceptionFromNative(exceptionCode)             {getThrowableExceptionObject(getCurrentJavaThread(), exceptionCode, getPCNative(), getFPNative()); throwThrowableObjectFromNative(&getCurrentJavaThread()->pObjectToThrow, getFPNative(), getPCNative());}
#define throwErrorFromNativeAndReturn(errorCode)            {throwErrorFromNative(errorCode); return;}
#define throwExceptionFromNativeAndReturn(exceptionCode)    {throwExceptionFromNative(exceptionCode); return;}


#define pushStackFrameNative(pMethodDef) {                                                      \
    FRAME pNewFrame;                                                                            \
                                                                                                \
    pushFrame(pNewFrame, pMethodDef, getSPNative(), getFPNative(), getPCNative(),               \
        getStackLimitNative(), throwErrorFromNativeAndReturn);                                  \
    setRegistersNative(pNewFrame, pMethodDef);                                                  \
}


/* We must push the internal frame for clinit, 
 * then push the arguments as they appear in the above structure.
 */
#define initializeClassNative(pClassInstance)                                                   \
    pushStackFrameNative(pInternalClinitMethod);                                                \
    pushOperandStackObjectNative(NULL);                                                         \
    pushOperandStackObjectNative(getClassObjectFromClassInstance(pClassInstance));              \
    pushOperandStackIntNative(CLINIT_INITIAL_STAGE);





void javaLangThreadStart();
void javaLangThreadYield();
void javaLangThreadJoin();
void javaLangThreadIsAlive();
void javaLangThreadGetPriority();
void javaLangThreadSetPriority();
void javaLangThreadSleep();
void javaLangThreadActiveCount();
void javaLangThreadCurrentThread();
void javaLangThreadInterrupt();
void javaLangObjectWait();
void javaLangObjectWaitJI();
void javaLangObjectWaitJ();
void javaLangObjectNotify();
void javaLangObjectNotifyAll();
void javaLangObjectHashCode();
void javaLangObjectGetClass();
void javaLangClassIsInterface();
void javaLangClassIsArray();
void javaLangClassIsInstance();
void javaLangClassIsAssignableFrom();
void javaLangClassGetName();
void javaLangClassNewInstance();
void javaLangClassForName();
void javaLangClassGetResourceAsStream();
void javaLangThrowableCreateStackTrace();
void javaLangThrowablePrintStackTrace();
void javaLangSystemGetProperty();
void javaLangSystemCurrentTimeMillis();
void javaLangSystemArrayCopy();
void javaLangRuntimeExit();
void javaLangRuntimeGC();
void javaLangRuntimeFreeMemory();
void javaLangRuntimeTotalMemory();
void javaLangStringCharAtInternal();
void javaLangStringCharAt();
void javaLangIntegerValue();
void javaLangMathIMax();
void javaLangMathIMin();
void javaLangMathIAbs();
void javaLangMathLMax();
void javaLangMathLMin();
void javaLangMathLAbs();
void javaUtilVectorElementAtInternal();
void comMjvmkStandardOutputStreamWrite();
void comMjvmkStandardOutputStreamFlush();
void comMjvmkStandardErrorStreamWrite();
void comMjvmkStandardOutputStreamWriteBytes();
void comMjvmkStandardOutputStreamWriteByteArray();
void comMjvmkResourceInputStreamClose();
void comMjvmkResourceInputStreamRead();

#endif


