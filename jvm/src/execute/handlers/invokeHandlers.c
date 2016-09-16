
#ifdef INSIDE_INSTRUCTION_LOOP

#define linkNativeMethod(pMethodDef) {                                                              \
    pNativeMethodDef = (NATIVE_METHOD_DEF) pMethodDef;                                              \
    ret = getNativeFunctionIndex(pNativeMethodDef->base.pOwningClass->header.key.namePackageKey,    \
                pNativeMethodDef->base.nameType.nameTypeKey,                                        \
                &pNativeMethodDef->nativeFunctionIndex);                                            \
    if(ret != SUCCESS) {                                                                            \
        throwErrorAndBreak(ret);                                                                    \
    }                                                                                               \
    setNativeMethodLinked(pNativeMethodDef);                                                        \
}


#define callNative(index) getNativeFunction(index)()

#define callNativeFunction(pMethodDef) callNative(((NATIVE_METHOD_DEF) pMethodDef)->nativeFunctionIndex)


#if EARLY_NATIVE_LINKING
#define checkNativeLinking(pMethodDef)
#else
#define checkNativeLinking(pMethodDef)                                                              \
    if(isNativeMethod(pMethodDef) && !isNativeMethodLinked((NATIVE_METHOD_DEF) pMethodDef))         \
        linkNativeMethod(pMethodDef)
#endif





#define resolveMethodRef(pConstantPool, constantPoolIndex, isVirtual, isStatic) {                   \
    /* get the method def using the constant pool reference */                                      \
    if(constantPoolMethodIsResolved(pConstantPool, constantPoolIndex)) {                            \
        pMethodDef = getConstantPoolMethodDef(pConstantPool, constantPoolIndex);                    \
    }                                                                                               \
    else {                                                                                          \
        /* the following function will alter the constant pool */                                   \
        ret = resolveConstantPoolMethodRef(pConstantPool, constantPoolIndex, getCurrentClassDef(),  \
            isStatic, isVirtual, &pMethodDef);                                                      \
        if(ret != SUCCESS) {                                                                        \
            throwErrorAndBreak(ret);                                                                \
        }                                                                                           \
    }                                                                                               \
}




/* invokeinterface */


START_INSTRUCTION_HANDLER(INVOKEINTERFACE)
    instructionRead(opsArray, getPC(), 5);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);

    if(constantPoolInterfaceMethodIsResolved(getCurrentConstantPool(), index)) {
        pInterfaceMethodDef = getConstantPoolInterfaceMethodDef(getCurrentConstantPool(), index);
    }
    else {
        ret = resolveConstantPoolInterfaceMethodRef(getCurrentConstantPool(), index, getCurrentClassDef(), &pInterfaceMethodDef);
        if(ret != SUCCESS) {
            /* try finding the method in java.lang.Object before we give up */
            nameType = getConstantPoolNameType(getCurrentConstantPool(), getConstantPoolInterfaceMethod(getCurrentConstantPool(), index)->nameTypeIndex);
            ret = lookupMethod(pJavaLangObjectDef, nameType, TRUE, &pMethodDef);
            if(ret != SUCCESS) {
                throwErrorAndBreak(ret);
            }
            pObject = getOperandStackObject(pMethodDef->base.parameterVarCount - 1);
            if(pObject == NULL) {
                throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
            }
            goto callInterfaceMethod;
        }
    }
    quickenInstructionStream5X2X2(getPC(), INVOKEINTERFACE_QUICK, 
        getClassTableIndexFromClass(pInterfaceMethodDef->base.pOwningClass), 
        pInterfaceMethodDef->base.methodIndex);
    goto findInterfaceMethodDef;
END_HANDLER



START_INSTRUCTION_HANDLER(INVOKEINTERFACE_QUICK)
    index = getU2(getPC() + 1);
    pClassInstance = getClassInTableByIndex(index);
    index = getU2(getPC() + 3);
    pInterfaceMethodDef = (INTERFACE_METHOD_DEF) getInstanceMethod(pClassInstance->pRepresentativeClassDef, index);
    
findInterfaceMethodDef:

    /* we have the interface method definition, we need to find the actual method in the object */
    pObject = getOperandStackObject(pInterfaceMethodDef->base.parameterVarCount - 1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    LOG_METHOD_CALL("invoke interface", (METHOD_DEF) pInterfaceMethodDef, pObject);

    /* get the interface list from the object */
    pClassDef = (LOADED_CLASS_DEF) pObject->header.pClassDef;
    pInterfaceList = pClassDef->loadedHeader.pSuperInterfaceList;
    if(pInterfaceList == NULL) {
        throwErrorAndBreak(ERROR_CODE_INCOMPATIBLE_CLASS_CHANGE);
    }

    /* find the index of the implemented interface */
    namePackage.namePackageKey = getNamePackageKey(pInterfaceMethodDef->base.pOwningClass);
    for(index=0; index < pInterfaceList->length; index++) {
        if(namePackage.namePackageKey == getNamePackageKey(pInterfaceList->pInterfaceDefs[index])) {
            pMethodDef = pInterfaceList->pppInterfaceImplementationTable[index][pInterfaceMethodDef->base.methodIndex];
            goto callInterfaceMethod;
        }
    }
    throwErrorAndBreak(ERROR_CODE_INCOMPATIBLE_CLASS_CHANGE);

callInterfaceMethod:
    LOG_METHOD_CALL("interface call", pMethodDef, pObject);
    
    if(pMethodDef == NULL || isAbstractMethod(pMethodDef)) {
        throwErrorAndBreak(ERROR_CODE_ILLEGAL_ACCESS);
    }
    if(!isPublicMethod(pMethodDef)) {
        throwErrorAndBreak(ERROR_CODE_ABSTRACT_METHOD);    
    }    
    checkNativeLinking(pMethodDef);
    instructionSize = 5;
    goto callInstanceMethod;
END_HANDLER



/* invokevirtual */



START_INSTRUCTION_HANDLER(INVOKEVIRTUAL)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveMethodRef(getCurrentConstantPool(), index, TRUE, FALSE);
#if QUICKENING_ENABLED
    index2 = getClassTableIndexFromClass(pMethodDef->base.pOwningClass);
    index = pMethodDef->base.methodIndex;
    if(index2 < 256 && index < 256) {
        quickenInstructionStream3(getPC(), INVOKEVIRTUAL_QUICK, (UINT8) index2, (UINT8) index);
    }
#endif
    goto callVirtualMethod;
END_HANDLER



START_INSTRUCTION_HANDLER(INVOKEVIRTUAL_QUICK)
    index = getPC()[1];
    pClassInstance = getClassInTableByIndex(index);
    index = getPC()[2];
    pMethodDef = getInstanceMethod(pClassInstance->pRepresentativeClassDef, index);
    
callVirtualMethod: /* we've found the virtual base method, now find the actual method */
    pObject = getOperandStackObject(pMethodDef->base.parameterVarCount - 1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    pClassDef = (LOADED_CLASS_DEF) pObject->header.pClassDef;
    LOG_METHOD_CALL("invoke virtual", pMethodDef, pObject);
    LOG_METHOD_CALL("virtual call", getInstanceMethod(pClassDef, pMethodDef->base.methodIndex), pObject);
    pMethodDef = getInstanceMethod(pClassDef, pMethodDef->base.methodIndex);
    if(isAbstractMethod(pMethodDef)) {
        throwErrorAndBreak(ERROR_CODE_ABSTRACT_METHOD);
    }
    checkNativeLinking(pMethodDef);
    instructionSize = 3;

callInstanceMethod:

    if(isNativeMethod(pMethodDef)) {
        if(isSynchronizedMethod(pMethodDef)) {
            pMonitor = getObjectMonitor(pObject);
            if(pMonitor == NULL) {
                ppMonitor = getObjectMonitorPtr(pObject);
                ret = collectibleMemoryCallocMonitor(ppMonitor);
                if(ret != SUCCESS) {
                    throwErrorAndBreak(ret);
                }
                pMonitor = *ppMonitor;
            }
            monitorEnterConditionalNative(pMonitor);
            callNativeFunction(pMethodDef);
            monitorExit(pMonitor);
        }
        else {
            callNativeFunction(pMethodDef);
        }
        checkforNativeJavaRegisterSwitch();
    }
    else {
        if(isSynchronizedMethod(pMethodDef)) {
            incrementProgramCounterBy(instructionSize);
            pushStackFrame(pMethodDef);
            incrementOperandStack(); /* pushStackFrame assumes non-synchronized, so save a space for the monitor  */
            pMonitor = getObjectMonitor(pObject);
            if(pMonitor == NULL) {
                ppMonitor = getObjectMonitorPtr(pObject);
                ret = collectibleMemoryCallocMonitor(ppMonitor);
                if(ret != SUCCESS) {
                    throwErrorAndBreak(ret);
                }
                pMonitor = *ppMonitor;
            }
            monitorEnter(pMonitor);
            setMonitorInFrame(getFP(), pMonitor);
            checkForMonitorBlocked(pMonitor); /* will switch threads if necessary */
        }
        else {
            /* the frame registers are updated by pushFrame */
            incrementProgramCounterBy(instructionSize);
            pushStackFrame(pMethodDef); /* note: may fail due to stack overflow */
        }
    }
    incrementSwitchTimeCounter();
END_HANDLER





/* invokestatic */



START_INSTRUCTION_HANDLER(INVOKESTATIC)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);

    resolveMethodRef(getCurrentConstantPool(), index, FALSE, TRUE);

    pClassInstance = getClassInstanceFromClassDef(pMethodDef->base.pOwningClass);
    if(!classIsInitialized(pClassInstance, getCurrentJavaThread())) {
        initializeClass(pClassInstance);
    }
    checkNativeLinking(pMethodDef);

    

#if QUICKENING_ENABLED
    if(isSynchronizedMethod(pMethodDef)) {
        index2 = getClassTableIndexFromClass(pMethodDef->base.pOwningClass);
        index = pMethodDef->base.methodIndex;
        if(index2 < 256 && index < 256) {
            if(getClassMonitor(pClassInstance) == NULL) {
                ret = memoryCallocMonitor(getClassMonitorPtr(pClassInstance));
                if(ret != SUCCESS) {
                    throwErrorAndBreak(ret);
                }
            }
            quickenInstructionStream3(getPC(), isNativeMethod(pMethodDef) ? INVOKESTATIC_QUICK_NATIVE_SYNC : INVOKESTATIC_QUICK_SYNC, (UINT8) index2, (UINT8) index);
            instructionBreak();
        }
    }
    else {
        if(isNativeMethod(pMethodDef)) {
            quickenInstructionStream3X2(getPC(), INVOKESTATIC_QUICK_NATIVE, ((NATIVE_METHOD_DEF) pMethodDef)->nativeFunctionIndex);
            instructionBreak();
        }
        else {
            index = pMethodDef->base.methodIndex;
            index2 = getClassTableIndexFromClass(pMethodDef->base.pOwningClass);
            if(index2 < 256 && index < 256) {
                quickenInstructionStream3(getPC(), INVOKESTATIC_QUICK, (UINT8) index2, (UINT8) index);
                instructionBreak();
            }
        
        }
    }
#endif
    LOG_METHOD("invoke static", pMethodDef);
    
    if(isNativeMethod(pMethodDef)) {
        if(isSynchronizedMethod(pMethodDef)) {
            pMonitor = getClassMonitor(pClassInstance);
            if(pMonitor == NULL) {
                ppMonitor = getClassMonitorPtr(pClassInstance);
                ret = memoryCallocMonitor(ppMonitor);
                if(ret != SUCCESS) {
                    throwErrorAndBreak(ret);
                }
                pMonitor = *ppMonitor;
            }
            monitorEnterConditionalNative(pMonitor);
            callNativeFunction(pMethodDef);
            monitorExit(pMonitor);
        }
        else {
            callNativeFunction(pMethodDef);
        }
        checkforNativeJavaRegisterSwitch();
    }
    else {
        if(isSynchronizedMethod(pMethodDef)) {
            incrementProgramCounterBy(3);
            pushStackFrame(pMethodDef);
            incrementOperandStack(); /* pushStackFrame assumes non-synchronized, so save a space for the monitor  */
            pMonitor = getClassMonitor(pClassInstance);
            if(pMonitor == NULL) {
                ppMonitor = getClassMonitorPtr(pClassInstance);
                ret = memoryCallocMonitor(ppMonitor);
                if(ret != SUCCESS) {
                    throwErrorAndBreak(ret);
                }
                pMonitor = *ppMonitor;
            }
            monitorEnter(pMonitor);
            setMonitorInFrame(getFP(), pMonitor);
            checkForMonitorBlocked(pMonitor); /* will switch threads if necessary */
        }
        else {
            incrementProgramCounterBy(3);
            pushStackFrame(pMethodDef); /* note: may fail due to stack overflow */
        }
    }
    incrementSwitchTimeCounter();
END_HANDLER





START_INSTRUCTION_HANDLER(INVOKESTATIC_QUICK)
    pClassInstance = getClassInTableByIndex(getPC()[1]);
    pMethodDef = getStaticMethod(pClassInstance->pRepresentativeClassDef, getPC()[2]);
    LOG_METHOD("invoke static", pMethodDef);
    incrementProgramCounterBy(3);
    pushStackFrame(pMethodDef);
    incrementSwitchTimeCounter();
END_HANDLER



START_INSTRUCTION_HANDLER(INVOKESTATIC_QUICK_NATIVE)
    callNative(getU2(getPC() + 1));
    checkforNativeJavaRegisterSwitch();
    incrementSwitchTimeCounter();
END_HANDLER



START_INSTRUCTION_HANDLER(INVOKESTATIC_QUICK_SYNC)
    pClassInstance = getClassInTableByIndex(getPC()[1]);
    pMethodDef = getStaticMethod(pClassInstance->pRepresentativeClassDef, getPC()[2]);
    pMonitor = getClassMonitor(pClassInstance);
    monitorEnter(pMonitor);
    LOG_METHOD("invoke static", pMethodDef);
    incrementProgramCounterBy(3);
    pushStackFrame(pMethodDef);
    incrementOperandStack();
    setMonitorInFrame(getFP(), pMonitor);
    checkForMonitorBlocked(pMonitor);
    incrementSwitchTimeCounter();
END_HANDLER



START_INSTRUCTION_HANDLER(INVOKESTATIC_QUICK_NATIVE_SYNC)
    pClassInstance = getClassInTableByIndex(getPC()[1]);
    pMethodDef = getStaticMethod(pClassInstance->pRepresentativeClassDef, getPC()[2]);
    pMonitor = getClassMonitor(pClassInstance);
    monitorEnterConditionalNative(pMonitor);
    LOG_METHOD("invoke static native", pMethodDef);
    callNativeFunction(pMethodDef);
    monitorExit(pMonitor);
    checkforNativeJavaRegisterSwitch();
    incrementSwitchTimeCounter();
END_HANDLER


/* invokespecial */


START_INSTRUCTION_HANDLER(INVOKESPECIAL)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveMethodRef(getCurrentConstantPool(), index, FALSE, FALSE);

    /* special super lookup for ACC_SUPER classes, see VM spec for invokesuper for details */
    if(isSuperSpecialClass(getCurrentClassDef()) && pMethodDef->base.nameType.nt.nameKey != initNameType.nt.nameKey) {
        if(isParentClass((COMMON_CLASS_DEF) pMethodDef->base.pOwningClass, (COMMON_CLASS_DEF) getCurrentClassDef())) {
            pMethodDef = getInstanceMethod(getCurrentClassDef()->header.pSuperClass, pMethodDef->base.methodIndex);
        }
    }
    checkNativeLinking(pMethodDef);

    

#if QUICKENING_ENABLED
    index2 = getClassTableIndexFromClass(pMethodDef->base.pOwningClass);
    index = pMethodDef->base.methodIndex;
    if(index2 < 256 && index < 256) {
        if(isSynchronizedMethod(pMethodDef)) {
            quickenInstructionStream3(getPC(), isNativeMethod(pMethodDef) ? INVOKESPECIAL_QUICK_NATIVE_SYNC : INVOKESPECIAL_QUICK_SYNC, (UINT8) index2, (UINT8) index);
        }
        else {
            quickenInstructionStream3(getPC(), isNativeMethod(pMethodDef) ? INVOKESPECIAL_QUICK_NATIVE : INVOKESPECIAL_QUICK, (UINT8) index2, (UINT8) index);
        }
        instructionBreak(); /* since we've already tested for native and synchronized, execute the quickened instruction */
    }
#endif
    pObject = getOperandStackObject(pMethodDef->base.parameterVarCount - 1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    LOG_METHOD("invoke special", pMethodDef);


    instructionSize = 3;
    goto callInstanceMethod;
END_HANDLER



START_INSTRUCTION_HANDLER(INVOKESPECIAL_QUICK)
    pClassInstance = getClassInTableByIndex(getPC()[1]);
    pMethodDef = getInstanceMethod(pClassInstance->pRepresentativeClassDef, getPC()[2]);
    pObject = getOperandStackObject(pMethodDef->base.parameterVarCount - 1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    LOG_METHOD("invoke special", pMethodDef);
    incrementProgramCounterBy(3);
    pushStackFrame(pMethodDef);
    incrementSwitchTimeCounter();
END_HANDLER



START_INSTRUCTION_HANDLER(INVOKESPECIAL_QUICK_NATIVE)
    pClassInstance = getClassInTableByIndex(getPC()[1]);
    pMethodDef = getInstanceMethod(pClassInstance->pRepresentativeClassDef, getPC()[2]);
    pObject = getOperandStackObject(pMethodDef->base.parameterVarCount - 1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    LOG_METHOD("invoke special native", pMethodDef);
    callNativeFunction(pMethodDef);
    checkforNativeJavaRegisterSwitch();
    incrementSwitchTimeCounter();
END_HANDLER


START_INSTRUCTION_HANDLER(INVOKESPECIAL_QUICK_SYNC)
    pClassInstance = getClassInTableByIndex(getPC()[1]);
    pMethodDef = getInstanceMethod(pClassInstance->pRepresentativeClassDef, getPC()[2]);
    pObject = getOperandStackObject(pMethodDef->base.parameterVarCount - 1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    LOG_METHOD("invoke special", pMethodDef);
    if((pMonitor = getObjectMonitor(pObject)) == NULL) {
        ret = collectibleMemoryCallocMonitor(getObjectMonitorPtr(pObject));
        if(ret != SUCCESS) {
            throwErrorAndBreak(ret);
        }
        pMonitor = getObjectMonitor(pObject);
    }
    monitorEnter(pMonitor);
    incrementProgramCounterBy(3);
    pushStackFrame(pMethodDef);
    incrementOperandStack();
    setMonitorInFrame(getFP(), pMonitor);
    checkForMonitorBlocked(pMonitor);
    incrementSwitchTimeCounter();
END_HANDLER



START_INSTRUCTION_HANDLER(INVOKESPECIAL_QUICK_NATIVE_SYNC)
    pClassInstance = getClassInTableByIndex(getPC()[1]);
    pMethodDef = getInstanceMethod(pClassInstance->pRepresentativeClassDef, getPC()[2]);
    pObject = getOperandStackObject(pMethodDef->base.parameterVarCount - 1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    LOG_METHOD("invoke special native", pMethodDef);
    if((pMonitor = getObjectMonitor(pObject)) == NULL) {
        ret = collectibleMemoryCallocMonitor(getObjectMonitorPtr(pObject));
        if(ret != SUCCESS) {
            throwErrorAndBreak(ret);
        }
        pMonitor = getObjectMonitor(pObject);
    }
    monitorEnterConditionalNative(pMonitor);
    callNativeFunction(pMethodDef);
    monitorExit(pMonitor);
    checkforNativeJavaRegisterSwitch();
    incrementSwitchTimeCounter();
END_HANDLER





/* all the return instructions */


#define popCurrentFrameReturnInt(retValue) {                                                    \
    popFrame(getFP(), getLP(), 1);                                                              \
    pushOperandStackInt(retValue);                                                              \
}

/* setting the return value first will overwrite the first value in the frame, pMethod,
 * but this is OK since it is not used when popping the frame.  We need to set the return value
 * first so it is not garbage collected.
 */
#define popCurrentFrameReturnObject(retObject) {                                                \
    setStackFieldObject(getPreviousSP(getFP(), getLP()) + 1, retObject);                        \
    popFrame(getFP(), getLP(), 1);                                                              \
}

/* locals, fields and stack values are big endian, so the high byte is first */

#define popCurrentFrameReturnLong(retValue) {                                                   \
    popFrame(getFP(), getLP(), 2);                                                              \
    pushOperandStackLong(retValue);                                                             \
}

#if IMPLEMENTS_FLOAT

#define popCurrentFrameReturnFloat(retValue) {                                                  \
    popFrame(getFP(), getLP(), 1);                                                              \
    pushOperandStackFloat(retValue);                                                            \
}

#define popCurrentFrameReturnDouble(retDouble) {                                                \
    popFrame(getFP(), getLP(), 2);                                                              \
    pushOperandStackDouble(retDouble);                                                          \
}

#endif




START_INSTRUCTION_HANDLER(IRETURN)
    if(isSynchronizedMethod(getCurrentMethodDef())) {
        exitSynchronizedMonitorInFrame(getFP());
#if QUICKENING_ENABLED
        *getPC() = IRETURN_QUICK_SYNC;
    }
    else {
        *getPC() = IRETURN_QUICK_NON_SYNC;
#endif
    }
    intValue = getOperandStackInt(0);
    popCurrentFrameReturnInt(intValue);
    incrementSwitchTimeCounter();
END_HANDLER


START_INSTRUCTION_HANDLER(ARETURN)
    if(isSynchronizedMethod(getCurrentMethodDef())) {
        exitSynchronizedMonitorInFrame(getFP());
#if QUICKENING_ENABLED
        *getPC() = ARETURN_QUICK_SYNC;
    }
    else {
        *getPC() = ARETURN_QUICK_NON_SYNC;
#endif
    }
    pObject = getOperandStackObject(0);
    popCurrentFrameReturnObject(pObject);
    incrementSwitchTimeCounter();
END_HANDLER



START_INSTRUCTION_HANDLER(LRETURN)
    if(isSynchronizedMethod(getCurrentMethodDef())) {
        exitSynchronizedMonitorInFrame(getFP());
#if QUICKENING_ENABLED
        *getPC() = LRETURN_QUICK_SYNC;
    }
    else {
        *getPC() = LRETURN_QUICK_NON_SYNC;
#endif
    }
    longValue = getOperandStackLong(1);
    popCurrentFrameReturnLong(longValue);
    incrementSwitchTimeCounter();
END_HANDLER



START_INSTRUCTION_HANDLER(RETURN)
    if(isSynchronizedMethod(getCurrentMethodDef())) {
        exitSynchronizedMonitorInFrame(getFP());
#if QUICKENING_ENABLED
        *getPC() = RETURN_QUICK_SYNC;
    }
    else {
        *getPC() = RETURN_QUICK_NON_SYNC;
#endif
    }
    popCurrentFrame();
    incrementSwitchTimeCounter();
END_HANDLER




START_INSTRUCTION_HANDLER(IRETURN_QUICK_SYNC)
    exitSynchronizedMonitorInFrame(getFP());
START_INSTRUCTION_HANDLER(IRETURN_QUICK_NON_SYNC)
    intValue = getOperandStackInt(0);
    popCurrentFrameReturnInt(intValue);
    incrementSwitchTimeCounter();
END_HANDLER
END_HANDLER


START_INSTRUCTION_HANDLER(RETURN_QUICK_SYNC)
    exitSynchronizedMonitorInFrame(getFP());
START_INSTRUCTION_HANDLER(RETURN_QUICK_NON_SYNC)
    popCurrentFrame();
    incrementSwitchTimeCounter();
END_HANDLER
END_HANDLER                          


START_INSTRUCTION_HANDLER(LRETURN_QUICK_SYNC)
    exitSynchronizedMonitorInFrame(getFP());
START_INSTRUCTION_HANDLER(LRETURN_QUICK_NON_SYNC)
    longValue = getOperandStackLong(1);
    popCurrentFrameReturnLong(longValue);
    incrementSwitchTimeCounter();
END_HANDLER
END_HANDLER

             
START_INSTRUCTION_HANDLER(ARETURN_QUICK_SYNC)
    exitSynchronizedMonitorInFrame(getFP());
START_INSTRUCTION_HANDLER(ARETURN_QUICK_NON_SYNC)
    pObject = getOperandStackObject(0);
    popCurrentFrameReturnObject(pObject);
    incrementSwitchTimeCounter();
END_HANDLER
END_HANDLER



#if IMPLEMENTS_FLOAT

START_INSTRUCTION_HANDLER(DRETURN)
    if(isSynchronizedMethod(getCurrentMethodDef())) {
        exitSynchronizedMonitorInFrame(getFP());
    }
    doubleValue = getOperandStackDouble(1);
    popCurrentFrameReturnDouble(doubleValue);
    incrementSwitchTimeCounter();
END_HANDLER


START_INSTRUCTION_HANDLER(FRETURN)
    if(isSynchronizedMethod(getCurrentMethodDef())) {
        exitSynchronizedMonitorInFrame(getFP());
    }
    floatValue = getOperandStackFloat(0);
    popCurrentFrameReturnFloat(floatValue);
    incrementSwitchTimeCounter();
END_HANDLER



#endif

#endif

