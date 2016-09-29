#ifndef FRAMEREGISTERS_H
#define FRAMEREGISTERS_H

#include "javaThread/javaThread.h"


/* frame register access */


#if LOCAL_FRAME_REGISTERS

#define getSP()                                 (localFrameRegisters.sp)
#define getFP()                                 (localFrameRegisters.fp)
#define getPC()                                 (localFrameRegisters.pc)
#define getLP()                                 (localFrameRegisters.lp)


#define setSP(newsp)                            (localFrameRegisters.sp = (newsp))
#define setFP(newfp)                            (localFrameRegisters.fp = (newfp))
#define setPC(newpc)                            (localFrameRegisters.pc = (newpc))
#define setLP(newlp)                            (localFrameRegisters.lp = (newlp))

#if USE_CLASS_REGISTERS

#define getCurrentConstantPool()                (localFrameRegisters.pCurrentConstantPool)
#define getCurrentClassDef()                    (localFrameRegisters.pCurrentClassDef)
#define getCurrentMethodDef()                   (localFrameRegisters.pCurrentMethodDef)

#define setCurrentConstantPool(pConstantPool)   (localFrameRegisters.pCurrentConstantPool = (pConstantPool))
#define setCurrentClassDef(pClassDef)           (localFrameRegisters.pCurrentClassDef = (pClassDef))
#define setCurrentMethodDef(pMethodDef)         (localFrameRegisters.pCurrentMethodDef = (pMethodDef)) 

#endif

#else


#define getSP()                                 (frameRegisters.sp)
#define getFP()                                 (frameRegisters.fp)
#define getPC()                                 (frameRegisters.pc)
#define getLP()                                 (frameRegisters.lp)


#define setSP(newsp)                            (frameRegisters.sp = (newsp))
#define setFP(newfp)                            (frameRegisters.fp = (newfp))
#define setPC(newpc)                            (frameRegisters.pc = (newpc))
#define setLP(newlp)                            (frameRegisters.lp = (newlp))


#if USE_CLASS_REGISTERS

#define getCurrentConstantPool()                (frameRegisters.pCurrentConstantPool)
#define getCurrentClassDef()                    (frameRegisters.pCurrentClassDef)
#define getCurrentMethodDef()                   (frameRegisters.pCurrentMethodDef)

#define setCurrentConstantPool(pConstantPool)   (frameRegisters.pCurrentConstantPool = (pConstantPool))
#define setCurrentClassDef(pClassDef)           (frameRegisters.pCurrentClassDef = (pClassDef))
#define setCurrentMethodDef(pMethodDef)         (frameRegisters.pCurrentMethodDef = (pMethodDef)) 

#endif


#endif

#if !USE_CLASS_REGISTERS

#define getCurrentMethodDef()                   (getFrameMethod(getFP()))
#define getCurrentClassDef()                    (getCurrentMethodDef()->base.pOwningClass)
#define getCurrentConstantPool()                (getCurrentClassDef()->loadedHeader.pConstantPool)

#define setCurrentConstantPool(pConstantPool)
#define setCurrentClassDef(pClassDef)
#define setCurrentMethodDef(pMethodDef) 

#endif








/* stack limit operations */



#if GROW_STACK
#define getStackLimit() (getFrameStackLimit(getFP()))
#else
#define getStackLimit() (getCurrentJavaThread()->pStackLimit)
#endif





/* program counter operations */


#define incrementProgramCounterBy(num) (getPC()+=(num))
#define incrementProgramCounter() incrementProgramCounterBy(1)




/* operand stack operations */


#define pushOperandStackEntry(x)                setStackFieldData(++getSP(), x)
#define pushOperandStackInt(x)                  setStackFieldInt(++getSP(), x)
#define pushOperandStackObject(x)               setStackFieldObject(++getSP(), x)
#define pushOperandStackRetAddress(x)           setStackFieldRetAddress(++getSP(), x)
#define pushOperandStackLong(x)                 {++getSP(); setStackFieldLong(getSP(), x); ++getSP();}

#define popOperandStackIntIntoObject(x)         {setPrimitiveFieldInt(x, getStackFieldInt(getSP())); getSP()--;}
#define popOperandStackObjectIntoObject(x)      {setObjectFieldObject(x, getStackFieldObject(getSP())); getSP()--;}
#define popOperandStackLongIntoObject(x)        {--getSP(); setPrimitiveFieldLong(x, getStackFieldLong(getSP())); getSP()--;}

#define popOperandStackIntIntoLocal(x)          {setStackFieldInt(x, getStackFieldInt(getSP())); getSP()--;}
#define popOperandStackRetAddressIntoLocal(x)   {setStackFieldRetAddress(x, getStackFieldRetAddress(getSP())); getSP()--;}
#define popOperandStackObjectIntoLocal(x)       {setStackFieldObject(x, getStackFieldObject(getSP())); getSP()--;}
#define popOperandStackLongIntoLocal(x)         {--getSP(); setStackFieldLong(x, getStackFieldLong(getSP())); getSP()--;}

#define pStackEntry(index)                      (getSP() - (index))

#define getOperandStackEntry(index)             getStackFieldData(pStackEntry(index))
#define getOperandStackObject(index)            getStackFieldObject(pStackEntry(index))
#define getOperandStackObjectPointer(index)     getStackFieldObjectPtr(pStackEntry(index))
#define getOperandStackInt(index)               getStackFieldInt(pStackEntry(index))
#define getOperandStackLong(index)              getStackFieldLong(pStackEntry(index))

#define setOperandStackEntry(index, value)      setStackFieldData(pStackEntry(index), value)
#define setOperandStackInt(index, value)        setStackFieldInt(pStackEntry(index), value)
#define setOperandStackObject(index, pObject)   setStackFieldObject(pStackEntry(index), pObject)
#define setOperandStackLong(index, value)       setStackFieldLong(pStackEntry(index), value)

#define decrementOperandStackBy(num)            (getSP()-=(num))
#define decrementOperandStack()                 (getSP()--)
#define incrementOperandStackBy(num)            (getSP()+=(num))
#define incrementOperandStack()                 (getSP()++)
#define getResetSP(fp, isSynchronizedFrame)     (((JSTACK_FIELD) (isSynchronizedFrame ? endOfSynchronizedFrame(fp) : endOfFrame(fp))) - 1)


#if IMPLEMENTS_FLOAT

#define pushOperandStackDouble(x)               {++getSP(); setStackFieldDouble(getSP(), x); getSP()++;}
#define pushOperandStackFloat(x)                setStackFieldFloat(++getSP(), x)

#define popOperandStackFloatIntoObject(x)       {setPrimitiveFieldFloat(x, getStackFieldFloat(getSP())); getSP()--;}
#define popOperandStackDoubleIntoObject(x)      {--getSP(); setPrimitiveFieldDouble(x, getStackFieldDouble(getSP())); getSP()--;}

#define popOperandStackFloatIntoLocal(x)        {setStackFieldFloat(x, getStackFieldFloat(getSP())); getSP()--;}
#define popOperandStackDoubleIntoLocal(x)       {--getSP(); setStackFieldDouble(x, getStackFieldDouble(getSP())); getSP()--;}

#define getOperandStackFloat(index)             getStackFieldFloat(pStackEntry(index))
#define getOperandStackDouble(index)            getStackFieldDouble(pStackEntry(index))

#define setOperandStackFloat(index, value)      setStackFieldFloat(pStackEntry(index), value)
#define setOperandStackDouble(index, value)     setStackFieldDouble(pStackEntry(index), value)

#endif






/* local variable operations */


#define pLocalEntry(index) (getLP() + (index))

#define getLocalRetAddress(index)               getStackFieldRetAddress(pLocalEntry(index))       
#define getLocalInt(index)                      getStackFieldInt(pLocalEntry(index))
#define getLocalLong(index)                     getStackFieldLong(pLocalEntry(index))
#define getLocalObject(index)                   getStackFieldObject(pLocalEntry(index))

#if IMPLEMENTS_FLOAT

#define getLocalFloat(index)                    getStackFieldFloat(pLocalEntry(index))
#define getLocalDouble(index)                   getStackFieldDouble(pLocalEntry(index))

#endif





/* reading instructions from the byte code stream */


#define instructionRead5(pByte, pc)             {*(pByte) = *(pc); instructionRead4(pByte + 1, pc + 1);}
#define instructionRead4(pByte, pc)             {*(pByte) = *(pc); instructionRead3(pByte + 1, pc + 1);}
#define instructionRead3(pByte, pc)             {*(pByte) = *(pc); instructionRead2(pByte + 1, pc + 1);}
#define instructionRead2(pByte, pc)             {*(pByte) = *(pc); instructionRead1(pByte + 1, pc + 1);}
#define instructionRead1(pByte, pc)             (*(pByte) = *(pc))

/* atomic instruction read will read all byte of an instruction without interrupt (so the code stream cannot be altered while reading it) */

#define atomicInstructionRead(pByte, pc, length) enterCritical(); instructionRead##length(pByte, pc); exitCritical()

#define instructionSet1(pc, newInstruction)                                     (pc)[0] = (newInstruction)
#define instructionSet2(pc, newInstruction, arg1)                               {(pc)[1] = (arg1); instructionSet1(pc, newInstruction);}
#define instructionSet3(pc, newInstruction, arg1, arg2)                         {(pc)[2] = (arg2);  instructionSet2(pc, newInstruction, arg1);}
#define instructionSet3X2(pc, newInstruction, arg1)                             {(pc)[1] = ((arg1) >> 8); (pc)[2] = (byte) (arg1); instructionSet1(pc, newInstruction);}
#define instructionSet4(pc, newInstruction, arg1, arg2, arg3)                   {(pc)[3] = (arg3);  instructionSet3(pc, newInstruction, arg1, arg2);}
#define instructionSet5(pc, newInstruction, arg1, arg2, arg3, arg4)             {(pc)[4] = (arg4);  instructionSet4(pc, newInstruction, arg1, arg2, arg3);}
#define instructionSet5X2X2(pc, newInstruction, arg1, arg2)                     {(pc)[3] = ((arg2) >> 8); (pc)[4] = (byte) (arg2); instructionSet3X2(pc, newInstruction, arg1);}

#define atomicInstructionSet1(pc, newInstruction)                               {enterCritical(); instructionSet1(pc, newInstruction); exitCritical();}
#define atomicInstructionSet2(pc, newInstruction, arg1)                         {enterCritical(); instructionSet2(pc, newInstruction, arg1); exitCritical();}
#define atomicInstructionSet3(pc, newInstruction, arg1, arg2)                   {enterCritical(); instructionSet3(pc, newInstruction, arg1, arg2); exitCritical();}
#define atomicInstructionSet3X2(pc, newInstruction, arg1)                       {enterCritical(); instructionSet3X2(pc, newInstruction, arg1); exitCritical();}
#define atomicInstructionSet4(pc, newInstruction, arg1, arg2, arg3)             {enterCritical(); instructionSet4(pc, newInstruction, arg1, arg2, arg3); exitCritical();}
#define atomicInstructionSet5(pc, newInstruction, arg1, arg2, arg3, arg4)       {enterCritical(); instructionSet5(pc, newInstruction, arg1, arg2, arg3, arg4); exitCritical();}
#define atomicInstructionSet5X2X2(pc, newInstruction, arg1, arg2)               {enterCritical(); instructionSet5X2X2(pc, newInstruction, arg1, arg2); exitCritical();}





/* quickening macros */

#if QUICKENING_ENABLED

#if PREEMPTIVE_THREADS

#define instructionRead(pByte, pc, length)                                      atomicInstructionRead(pByte, pc, length)
#define quickenInstructionStream1(pc, newInstruction)                           instructionSet1(pc, newInstruction)
#define quickenInstructionStream2(pc, newInstruction, arg1)                     atomicInstructionSet2(pc, newInstruction, arg1)
#define quickenInstructionStream3(pc, newInstruction, arg1, arg2)               atomicInstructionSet3(pc, newInstruction, arg1, arg2)
#define quickenInstructionStream3X2(pc, newInstruction, arg1)                   atomicInstructionSet3X2(pc, newInstruction, arg1)
#define quickenInstructionStream5X2X2(pc, newInstruction, arg1, arg2)           atomicInstructionSet5X2X2(pc, newInstruction, arg1, arg2)

/* ensure the instruction has not just been quickened by another thread */
#define checkForQuickening(oldInstruction, newInstruction) if(oldInstruction != newInstruction) instructionBreak();

#else

#define instructionRead(pByte, pc, length)                                      instructionRead##length(pByte, pc)
#define quickenInstructionStream1(pc, newInstruction)                           instructionSet1(pc, newInstruction)
#define quickenInstructionStream2(pc, newInstruction, arg1)                     instructionSet2(pc, newInstruction, arg1)
#define quickenInstructionStream3(pc, newInstruction, arg1, arg2)               instructionSet3(pc, newInstruction, arg1, arg2)
#define quickenInstructionStream3X2(pc, newInstruction, arg1)                   instructionSet3X2(pc, newInstruction, arg1)
#define quickenInstructionStream5X2X2(pc, newInstruction, arg1, arg2)           instructionSet5X2X2(pc, newInstruction, arg1, arg2)

#define checkForQuickening(oldInstruction, newInstruction)

#endif

#else

#define instructionRead(pByte, pc, length)                                      instructionRead##length(pByte, pc)
#define quickenInstructionStream1(pc, newInstruction)
#define quickenInstructionStream2(pc, newInstruction, arg1)
#define quickenInstructionStream3(pc, newInstruction, arg1, arg2)
#define quickenInstructionStream3X2(pc, newInstruction, arg1)
#define quickenInstructionStream5X2X2(pc, newInstruction, arg1, arg2)


#define checkForQuickening(oldInstruction, newInstruction)


#endif





#endif