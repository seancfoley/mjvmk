
#ifdef INSIDE_INSTRUCTION_LOOP


#define resolveFieldRef(pConstantPool, constantPoolIndex, isModify, isStatic)                       \
    /* get the method def using the constant pool reference */                                      \
    if(constantPoolFieldIsResolved(pConstantPool, constantPoolIndex)) {                             \
        pFieldDef = getConstantPoolFieldDef(pConstantPool, constantPoolIndex);                      \
    }                                                                                               \
    else {                                                                                          \
        /* the following function will alter the constant pool */                                   \
        ret = resolveConstantPoolFieldRef(pConstantPool, constantPoolIndex, getCurrentClassDef(),   \
            isModify, isStatic, &pFieldDef);                                                        \
        if(ret != SUCCESS) {                                                                        \
            throwErrorAndBreak(ret);                                                                \
        }                                                                                           \
    }




START_INSTRUCTION_HANDLER(GETSTATIC)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveFieldRef(getCurrentConstantPool(), index, FALSE, TRUE);
    pClassInstance = getClassInstanceFromClassDef(pFieldDef->pOwningClass);
    if(!classIsInitialized(pClassInstance, getCurrentJavaThread())) {
        initializeClass(pClassInstance);
    }
    index = pFieldDef->fieldIndex;
#if QUICKENING_ENABLED
    if((index2 = getClassTableIndexFromClass(pFieldDef->pOwningClass)) < 256 && index < 256) {
        if(isPrimitiveField(pFieldDef)) {
            if(isDoubleField(pFieldDef)) {
                quickenInstructionStream3(getPC(), GETSTATIC_QUICK_DOUBLE_PRIMITIVE, (UINT8) index2, (UINT8) index);
                pushOperandStackLong(getPrimitiveFieldLong(getPrimitiveStaticField(pClassInstance, index)));
            }
            else {
                quickenInstructionStream3(getPC(), GETSTATIC_QUICK_PRIMITIVE, (UINT8) index2, (UINT8) index);
                pushOperandStackInt(getPrimitiveFieldInt(getPrimitiveStaticField(pClassInstance, index)));
            }
        }
        else {
            quickenInstructionStream3(getPC(), GETSTATIC_QUICK_OBJECT, (UINT8) index2, (UINT8) index);
            pushOperandStackObject(getObjectFieldObject(getObjectStaticField(pClassInstance, index)));
        }
    }
    else {
#endif
        if(isPrimitiveField(pFieldDef)) {
            if(isDoubleField(pFieldDef)) {
                pushOperandStackLong(getPrimitiveFieldLong(getPrimitiveStaticField(pClassInstance, index)));
            }
            else {
                pushOperandStackInt(getPrimitiveFieldInt(getPrimitiveStaticField(pClassInstance, index)));
            }
        }
        else {
            pushOperandStackObject(getObjectFieldObject(getObjectStaticField(pClassInstance, index)));
        }
#if QUICKENING_ENABLED
    }
#endif
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(GETSTATIC_QUICK_DOUBLE_PRIMITIVE)
    pushOperandStackLong(getPrimitiveFieldLong(getPrimitiveStaticField(getClassInTableByIndex(getPC()[1]), getPC()[2])));
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(GETSTATIC_QUICK_PRIMITIVE)
    pushOperandStackInt(getPrimitiveFieldInt(getPrimitiveStaticField(getClassInTableByIndex(getPC()[1]), getPC()[2])));
    incrementProgramCounterBy(3);
END_HANDLER

START_INSTRUCTION_HANDLER(GETSTATIC_QUICK_OBJECT)
    pushOperandStackObject(getObjectFieldObject(getObjectStaticField(getClassInTableByIndex(getPC()[1]), getPC()[2])));
    incrementProgramCounterBy(3);
END_HANDLER




START_INSTRUCTION_HANDLER(PUTSTATIC)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveFieldRef(getCurrentConstantPool(), index, TRUE, TRUE);
    pClassInstance = getClassInstanceFromClassDef(pFieldDef->pOwningClass);
    if(!classIsInitialized(pClassInstance, getCurrentJavaThread())) {
        initializeClass(pClassInstance);
    }
    index = pFieldDef->fieldIndex;
#if QUICKENING_ENABLED
    if((index2  = getClassTableIndexFromClass(pFieldDef->pOwningClass)) < 256 && index < 256) {
        if(isPrimitiveField(pFieldDef)) {
            if(isDoubleField(pFieldDef)) {
                quickenInstructionStream3(getPC(), PUTSTATIC_QUICK_DOUBLE_PRIMITIVE, (UINT8) index2, (UINT8) index);
                popOperandStackLongIntoObject(getPrimitiveStaticField(pClassInstance, index));
            }
            else {
                quickenInstructionStream3(getPC(), PUTSTATIC_QUICK_PRIMITIVE, (UINT8) index2, (UINT8) index);
                popOperandStackIntIntoObject(getPrimitiveStaticField(pClassInstance, index));
            }
        }
        else {
            quickenInstructionStream3(getPC(), PUTSTATIC_QUICK_OBJECT, (UINT8) index2, (UINT8) index);
            pObject = getOperandStackObject(0);
            concurrentGCGreyObject(pObject);
            setObjectFieldObject(getObjectStaticField(pClassInstance, index), pObject);
            decrementOperandStack();
        }
    }
    else {
#endif
        if(isPrimitiveField(pFieldDef)) {
            if(isDoubleField(pFieldDef)) {
                popOperandStackLongIntoObject(getPrimitiveStaticField(pClassInstance, index));
            }
            else {
                popOperandStackIntIntoObject(getPrimitiveStaticField(pClassInstance, index));
            }
        }
        else {
            pObject = getOperandStackObject(0);
            concurrentGCGreyObject(pObject);
            setObjectFieldObject(getObjectStaticField(pClassInstance, index), pObject);
            decrementOperandStack();
        }
#if QUICKENING_ENABLED
    }
#endif
    incrementProgramCounterBy(3);
END_HANDLER



START_INSTRUCTION_HANDLER(PUTSTATIC_QUICK_DOUBLE_PRIMITIVE)
    popOperandStackLongIntoObject(getPrimitiveStaticField(getClassInTableByIndex(getPC()[1]), getPC()[2]));
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(PUTSTATIC_QUICK_PRIMITIVE)
    popOperandStackIntIntoObject(getPrimitiveStaticField(getClassInTableByIndex(getPC()[1]), getPC()[2]));
    incrementProgramCounterBy(3);
END_HANDLER

START_INSTRUCTION_HANDLER(PUTSTATIC_QUICK_OBJECT)
    pObject = getOperandStackObject(0);
    concurrentGCGreyObject(pObject);
    setObjectFieldObject(getObjectStaticField(getClassInTableByIndex(getPC()[1]), getPC()[2]), pObject);
    decrementOperandStack();
    incrementProgramCounterBy(3);
END_HANDLER




START_INSTRUCTION_HANDLER(GETFIELD)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveFieldRef(getCurrentConstantPool(), index, FALSE, FALSE);
    index = pFieldDef->fieldIndex;
    pObject = getOperandStackObject(0);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    if(isPrimitiveField(pFieldDef)) {
        if(isDoubleField(pFieldDef)) {
            quickenInstructionStream3X2(getPC(), GETFIELD_QUICK_DOUBLE_PRIMITIVE, index);
            setOperandStackLong(0, getPrimitiveFieldLong(getPrimitiveField(pObject, index)));
            incrementOperandStack();
        }
        else {
            if(index == 0) {
                quickenInstructionStream1(getPC(), GETFIELD_0_QUICK_PRIMITIVE);
            }
            else {
                quickenInstructionStream3X2(getPC(), GETFIELD_QUICK_PRIMITIVE, index);
            }
            setOperandStackInt(0, getPrimitiveFieldInt(getPrimitiveField(pObject, index)));
        }
    }
    else {
        quickenInstructionStream3X2(getPC(), GETFIELD_QUICK_OBJECT, index);
        pObject = getObjectFieldObject(getObjectField(pObject, index));
        concurrentGCGreyObject(pObject);
        setOperandStackObject(0, pObject);
    }
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(GETFIELD_0_QUICK_PRIMITIVE)
    pObject = getOperandStackObject(0);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    setOperandStackInt(0, getPrimitiveFieldInt(getPrimitiveField(pObject, 0)));
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(GETFIELD_QUICK_DOUBLE_PRIMITIVE)
    pObject = getOperandStackObject(0);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    setOperandStackLong(0, getPrimitiveFieldLong(getPrimitiveField(pObject, getU2(getPC() + 1))));
    incrementOperandStack();
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(GETFIELD_QUICK_PRIMITIVE)
    pObject = getOperandStackObject(0);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    setOperandStackInt(0, getPrimitiveFieldInt(getPrimitiveField(pObject, getU2(getPC() + 1))));
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(GETFIELD_QUICK_OBJECT)
    pObject = getOperandStackObject(0);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    pObject = getObjectFieldObject(getObjectField(pObject, getU2(getPC() + 1)));
    concurrentGCGreyObject(pObject);
    setOperandStackObject(0, pObject);
    incrementProgramCounterBy(3);
END_HANDLER




START_INSTRUCTION_HANDLER(PUTFIELD)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveFieldRef(getCurrentConstantPool(), index, TRUE, FALSE);
    index = pFieldDef->fieldIndex;
    pObject = getOperandStackObject(1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    if(isPrimitiveField(pFieldDef)) {
        if(isDoubleField(pFieldDef)) {
            quickenInstructionStream3X2(getPC(), PUTFIELD_QUICK_DOUBLE_PRIMITIVE, index);
            popOperandStackLongIntoObject(getPrimitiveField(pObject, index));
        }
        else {
            quickenInstructionStream3X2(getPC(), PUTFIELD_QUICK_PRIMITIVE, index);
            popOperandStackIntIntoObject(getPrimitiveField(pObject, index));
        }
    }
    else {
        quickenInstructionStream3X2(getPC(), PUTFIELD_QUICK_OBJECT, index);
        popOperandStackObjectIntoObject(getObjectField(pObject, index));
    }
    decrementOperandStack();
    incrementProgramCounterBy(3);
END_HANDLER



START_INSTRUCTION_HANDLER(PUTFIELD_QUICK_DOUBLE_PRIMITIVE)
    pObject = getOperandStackObject(1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    popOperandStackLongIntoObject(getPrimitiveField(pObject, getU2(getPC() + 1)));
    decrementOperandStack();
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(PUTFIELD_QUICK_PRIMITIVE)
    pObject = getOperandStackObject(1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    popOperandStackIntIntoObject(getPrimitiveField(pObject, getU2(getPC() + 1)));
    decrementOperandStack();
    incrementProgramCounterBy(3);
END_HANDLER

START_INSTRUCTION_HANDLER(PUTFIELD_QUICK_OBJECT)
    pObject = getOperandStackObject(1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    popOperandStackObjectIntoObject(getObjectField(pObject, getU2(getPC() + 1)));
    decrementOperandStack();
    incrementProgramCounterBy(3);
END_HANDLER


#endif