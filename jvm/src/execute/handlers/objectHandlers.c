
#ifdef INSIDE_INSTRUCTION_LOOP



#define resolveClassRefInConstantPool(pConstantPool, constantPoolIndex)                             \
    if(constantPoolClassIsResolved(pConstantPool, constantPoolIndex)) {                             \
        pCommonClassDef = getConstantPoolClassDef(pConstantPool, constantPoolIndex);                \
    }                                                                                               \
    else {                                                                                          \
        /* the following function will alter the constant pool */                                   \
        ret = resolveConstantPoolClassRef(pConstantPool, constantPoolIndex,                         \
            getCurrentClassDef(), &pCommonClassDef);                                                \
        if(ret != SUCCESS) {                                                                        \
            throwErrorAndBreak(ret);                                                                \
        }                                                                                           \
    }

START_INSTRUCTION_HANDLER(NEW)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveClassRefInConstantPool(getCurrentConstantPool(), index);
    if(isAbstractClass(pCommonClassDef)) {
        throwErrorAndBreak(ERROR_CODE_INSTANTIATION);
    }
    index = getClassTableIndexFromClass(pCommonClassDef);
    pClassInstance = getClassInTableByIndex(index);
    if(!classIsInitialized(pClassInstance, getCurrentJavaThread())) {
        initializeClass(pClassInstance);
    }

#if QUICKENING_ENABLED
    quickenInstructionStream3X2(getPC(), NEW_QUICK, (UINT8) index);  
#endif
    goto createObject;
END_HANDLER


START_INSTRUCTION_HANDLER(NEW_QUICK)
    pClassInstance = getClassInTableByIndex(getU2(getPC() + 1));
createObject:
    LOG_CLASS_REF("instantiating instance", pClassInstance->pRepresentativeClassDef->key);
    incrementOperandStack();
    ret = instantiateCollectibleObject(pClassInstance, getOperandStackObjectPointer(0));
    if(ret != SUCCESS) {
        throwErrorAndBreak(ret);
    }
    LOG_OBJECT("instantiated", getOperandStackObject(0));

    /*
    {
        char *pkg; 
        char *name;
        UINT16 pkgLength;
        UINT16 nameLength;
        char formatString[25];

        pkg = getPackageNameStringByKey(pClassInstance->pRepresentativeClassDef->key.np.packageKey, &pkgLength);
        name = getUTF8StringByKey(pClassInstance->pRepresentativeClassDef->key.np.nameKey, &nameLength);
        if(strncmp("Test", name, 4)==0) {
            psPrintErr("instantiated ");
            psPrintNErr(name, nameLength);
            sprintf(formatString, " thread 0x%x object 0x%x\n", getCurrentThread(), getOperandStackObject(0));
            psPrintErr(formatString);
        }
    }
    */

    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(ANEWARRAY)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveClassRefInConstantPool(getCurrentConstantPool(), index);
   
    /* increase the array depth in the key and obtain the new array class instance */
    namePackage.np.packageKey = addArrayDepthToPackageKey(getPackageStringKey(pCommonClassDef->key.np.packageKey), getArrayDepth(pCommonClassDef->key.np.packageKey) + 1); 
    namePackage.np.nameKey = pCommonClassDef->key.np.nameKey;
    ret = obtainClassRef(namePackage, &pClassInstance);
    if(ret != SUCCESS) {
        throwErrorAndBreak(ret);
    }
    index = pClassInstance->pRepresentativeClassDef->tableIndex;
#if QUICKENING_ENABLED
    quickenInstructionStream3X2(getPC(), ANEWARRAY_QUICK, index);  
#endif
    goto createObjectArray;
END_HANDLER


START_INSTRUCTION_HANDLER(ANEWARRAY_QUICK)
    pClassInstance = getClassInTableByIndex(getU2(getPC() + 1));
createObjectArray:
    intValue = getOperandStackInt(0);
    if(((INT32) intValue) < 0) {
        throwExceptionAndBreak(EXCEPTION_CODE_NEGATIVE_ARRAY_SIZE);
    }
    LOG_CLASS_REF("instantiating instance", pClassInstance->pRepresentativeClassDef->key);
    ret = instantiateCollectibleObjectArray(pClassInstance, intValue, (ARRAY_INSTANCE *) getOperandStackObjectPointer(0));
    if(ret != SUCCESS) {
        throwErrorAndBreak(ret);
    }
    LOG_OBJECT("instantiated", getOperandStackObject(0));
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(MULTIANEWARRAY)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveClassRefInConstantPool(getCurrentConstantPool(), index);
    index = getClassTableIndexFromClass(pCommonClassDef);
    pClassInstance = getClassInTableByIndex(index);
#if QUICKENING_ENABLED
    quickenInstructionStream3X2(getPC(), MULTIANEWARRAY_QUICK, index);  
#endif
    goto createMultiObjectArray;
END_HANDLER


START_INSTRUCTION_HANDLER(MULTIANEWARRAY_QUICK)
    pClassInstance = getClassInTableByIndex(getU2(getPC() + 1));
createMultiObjectArray:
    LOG_CLASS_REF("instantiating instance", pClassInstance->pRepresentativeClassDef->key);
    intValue = getPC()[3]; /* get the dimension count */
    /* check the array sizes for negative values */
    ret = SUCCESS;
    for(i=0; i<intValue; i++) {
        if(0 > (INT32) getOperandStackInt(i)) {
            ret = EXCEPTION_CODE_NEGATIVE_ARRAY_SIZE;
            break;
        }
    }
    if(ret != SUCCESS) {
        throwErrorAndBreak(ret);
    }
    ret = instantiateCollectibleMultiArray(pClassInstance, intValue, pStackEntry(intValue - 1), 
        (ARRAY_INSTANCE *) getOperandStackObjectPointer(intValue - 1));
    if(ret != SUCCESS) {
        throwErrorAndBreak(ret);
    }
    if(intValue > 1) {
        decrementOperandStackBy(intValue - 1);
    }
    LOG_OBJECT("instantiated", getOperandStackObject(0));
    incrementProgramCounterBy(4);
END_HANDLER


START_INSTRUCTION_HANDLER(NEWARRAY)
    instructionRead(opsArray, getPC(), 2);
    checkForQuickening(instruction, opsArray[0]);
    switch(opsArray[1]) {
        case T_BOOLEAN: arrayTypeChars[1] = BOOLEAN_ARRAY_TYPE; break;
        case T_CHAR: arrayTypeChars[1] = CHAR_ARRAY_TYPE; break;
        case T_BYTE: arrayTypeChars[1] = BYTE_ARRAY_TYPE; break;
        case T_SHORT: arrayTypeChars[1] = SHORT_ARRAY_TYPE; break;
        case T_INT: arrayTypeChars[1] = INT_ARRAY_TYPE; break;
        case T_LONG: arrayTypeChars[1] = LONG_ARRAY_TYPE; break;
#if IMPLEMENTS_FLOAT
        case T_FLOAT: arrayTypeChars[1] = FLOAT_ARRAY_TYPE; break;
        case T_DOUBLE: arrayTypeChars[1] = DOUBLE_ARRAY_TYPE; break;
#endif
    }
    ret = getClassParams(arrayTypeChars, 2, &classID);
    if(ret != SUCCESS) {
        throwErrorAndBreak(ret);
    }
    ret = resolveClassRef(getCurrentClassDef(), classID.key, &pClassInstance);
    if(ret != SUCCESS) {
        throwErrorAndBreak(ret);
    }
    index = getClassTableIndexFromClass(pClassInstance->pRepresentativeClassDef);
#if QUICKENING_ENABLED
    if(index < 256) {
        quickenInstructionStream2(getPC(), NEWARRAY_QUICK, (UINT8) index);  
    }
#endif
    goto createPrimitiveArray;
END_HANDLER


START_INSTRUCTION_HANDLER(NEWARRAY_QUICK)
    pClassInstance = getClassInTableByIndex(getPC()[1]);
createPrimitiveArray:
    LOG_CLASS_REF("instantiating instance", pClassInstance->pRepresentativeClassDef->key);
    intValue = getOperandStackInt(0);
    if(((INT32) intValue) < 0) {
        throwExceptionAndBreak(EXCEPTION_CODE_NEGATIVE_ARRAY_SIZE);
    }
    ret = instantiateCollectiblePrimitiveArray(pClassInstance, intValue, (ARRAY_INSTANCE *) getOperandStackObjectPointer(0));
    if(ret != SUCCESS) {
        throwErrorAndBreak(ret);
    }
    LOG_OBJECT("instantiated", getOperandStackObject(0));
    incrementProgramCounterBy(2);
END_HANDLER


START_INSTRUCTION_HANDLER(LDC)
    pushOperandStackEntry(getConstantPoolValue(getCurrentConstantPool(), getPC()[1]));
    incrementProgramCounterBy(2);
END_HANDLER

START_INSTRUCTION_HANDLER(LDC_W)
    pushOperandStackEntry(getConstantPoolValue(getCurrentConstantPool(), getU2(getPC() + 1)));
    incrementProgramCounterBy(3);
END_HANDLER

START_INSTRUCTION_HANDLER(LDC2_W)
    index = getU2(getPC() + 1);
    pushOperandStackEntry(getConstantPoolValue(getCurrentConstantPool(), index));
    pushOperandStackEntry(getConstantPoolValue(getCurrentConstantPool(), index + 1));
    incrementProgramCounterBy(3);
END_HANDLER

START_INSTRUCTION_HANDLER(MONITORENTER)
    if((pObject = getOperandStackObject(0)) == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    if((pMonitor = getObjectMonitor(pObject)) == NULL) {
        if(pObject->header.pClassDef == (COMMON_CLASS_DEF) pJavaLangClassDef) {
            /* class objects' monitors are not collectible */
            ret = memoryCallocMonitor(getObjectMonitorPtr(pObject));
        }
        else {
            ret = collectibleMemoryCallocMonitor(getObjectMonitorPtr(pObject));
        }
        if(ret != SUCCESS) {
            throwErrorAndBreak(ret);
        }
        pMonitor = getObjectMonitor(pObject);
    }
    LOG_LINE(("Entering monitor %x", pMonitor));
    monitorEnter(pMonitor);
    decrementOperandStack();
    incrementProgramCounter();
    checkForMonitorBlocked(pMonitor);
END_HANDLER


START_INSTRUCTION_HANDLER(MONITOREXIT)
    if(getOperandStackObject(0) == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    if(monitorNotOwnedByCurrentJavaThread(getOperandStackObject(0)->header.pMonitor)) {
        throwExceptionAndBreak(EXCEPTION_CODE_ILLEGAL_MONITOR_STATE);
    }
    LOG_LINE(("Exiting monitor %x", getOperandStackObject(0)->header.pMonitor));
    monitorExit(getOperandStackObject(0)->header.pMonitor);
    decrementOperandStack();
    incrementProgramCounter();
    enterCritical();
exitCritical();


END_HANDLER


START_INSTRUCTION_HANDLER(ARRAYLENGTH)
    if(getOperandStackObject(0) == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    setOperandStackInt(0, getArrayLength(getOperandStackObject(0)));
    incrementProgramCounter();
END_HANDLER


START_INSTRUCTION_HANDLER(ATHROW)
    throwThrowableObject(&getOperandStackObject(0), getFP(), getPC());
END_HANDLER



START_INSTRUCTION_HANDLER(CHECKCAST)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveClassRefInConstantPool(getCurrentConstantPool(), index);
#if QUICKENING_ENABLED
    index = getClassTableIndexFromClass(pCommonClassDef);
    quickenInstructionStream3X2(getPC(), CHECKCAST_QUICK, index);
#endif
    if(getOperandStackObject(0) != NULL && !isObjectInstanceOf(pCommonClassDef, getOperandStackObject(0))) {
        throwExceptionAndBreak(EXCEPTION_CODE_CLASS_CAST);
    }
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(CHECKCAST_QUICK)
    index = getU2(getPC() + 1);
    pClassInstance = getClassInTableByIndex(index);
    pCommonClassDef = pClassInstance->pRepresentativeClassDef;
    if(getOperandStackObject(0) != NULL && !isObjectInstanceOf(pCommonClassDef, getOperandStackObject(0))) {
        throwExceptionAndBreak(EXCEPTION_CODE_CLASS_CAST);
    }
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(INSTANCEOF)
    instructionRead(opsArray, getPC(), 3);
    checkForQuickening(instruction, opsArray[0]);
    index = getU2(opsArray + 1);
    resolveClassRefInConstantPool(getCurrentConstantPool(), index);
#if QUICKENING_ENABLED
    index = getClassTableIndexFromClass(pCommonClassDef);
    quickenInstructionStream3X2(getPC(), INSTANCEOF_QUICK, index);
#endif
    if(getOperandStackObject(0) != NULL && isObjectInstanceOf(pCommonClassDef, getOperandStackObject(0))) {
        setOperandStackInt(0, 1);
    }
    else {
        setOperandStackInt(0, 0);
    }
    incrementProgramCounterBy(3);
END_HANDLER


START_INSTRUCTION_HANDLER(INSTANCEOF_QUICK)
    index = getU2(getPC() + 1);
    pClassInstance = getClassInTableByIndex(index);
    pCommonClassDef = pClassInstance->pRepresentativeClassDef;
    if(getOperandStackObject(0) != NULL && isObjectInstanceOf(pCommonClassDef, getOperandStackObject(0))) {
        setOperandStackInt(0, 1);
    }
    else {
        setOperandStackInt(0, 0);
    }
    incrementProgramCounterBy(3);
END_HANDLER



START_INSTRUCTION_HANDLER(AALOAD)
    pObject = getOperandStackObject(1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    intValue = getOperandStackInt(0);
    if(((INT32) intValue) < 0 || intValue >= getArrayLength(pObject)) {
        throwExceptionAndBreak(EXCEPTION_CODE_ARRAY_INDEX_OUT_OF_BOUNDS);
    }
    pObject = getObjectArrayElement(pObject, intValue);
    concurrentGCGreyObject(pObject);
    setOperandStackObject(1, pObject);
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

#define ARRAY_LOAD_HANDLER(prefix, type, arrayType)                             \
START_INSTRUCTION_HANDLER(prefix##ALOAD)                                        \
    pObject = getOperandStackObject(1);                                         \
    if(pObject == NULL) {                                                       \
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);                    \
    }                                                                           \
    intValue = getOperandStackInt(0);                                           \
    if(((INT32) intValue) < 0 || intValue >= getArrayLength(pObject)) {         \
        throwExceptionAndBreak(EXCEPTION_CODE_ARRAY_INDEX_OUT_OF_BOUNDS);       \
    }                                                                           \
    setOperandStack##type(1, get##arrayType##ArrayElement(pObject, intValue));  \
    decrementOperandStack();                                                    \
    incrementProgramCounter();                                                  \
END_HANDLER


#define DOUBLE_ARRAY_LOAD_HANDLER(prefix, type, arrayType)                      \
START_INSTRUCTION_HANDLER(prefix##ALOAD)                                        \
    pObject = getOperandStackObject(1);                                         \
    if(pObject == NULL) {                                                       \
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);                    \
    }                                                                           \
    intValue = getOperandStackInt(0);                                           \
    if(((INT32) intValue) < 0 || intValue >= getArrayLength(pObject)) {         \
        throwExceptionAndBreak(EXCEPTION_CODE_ARRAY_INDEX_OUT_OF_BOUNDS);       \
    }                                                                           \
    setOperandStack##type(1, get##arrayType##ArrayElement(pObject, intValue));  \
    incrementProgramCounter();                                                  \
END_HANDLER

#define ARRAY_STORE_HANDLER(prefix, type, arrayType)                            \
START_INSTRUCTION_HANDLER(prefix##ASTORE)                                       \
    pObject = getOperandStackObject(2);                                         \
    if(pObject == NULL) {                                                       \
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);                    \
    }                                                                           \
    intValue = getOperandStackInt(1);                                           \
    if(((INT32) intValue) < 0 || intValue >= getArrayLength(pObject)) {         \
        throwExceptionAndBreak(EXCEPTION_CODE_ARRAY_INDEX_OUT_OF_BOUNDS);       \
    }                                                                           \
    set##arrayType##ArrayElement(pObject, intValue, getOperandStack##type(0));  \
    decrementOperandStackBy(3);                                                 \
    incrementProgramCounter();                                                  \
END_HANDLER


#define DOUBLE_ARRAY_STORE_HANDLER(prefix, type, arrayType)                     \
START_INSTRUCTION_HANDLER(prefix##ASTORE)                                       \
    pObject = getOperandStackObject(3);                                         \
    if(pObject == NULL) {                                                       \
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);                    \
    }                                                                           \
    intValue = getOperandStackInt(2);                                           \
    if(((INT32) intValue) < 0 || intValue >= getArrayLength(pObject)) {         \
        throwExceptionAndBreak(EXCEPTION_CODE_ARRAY_INDEX_OUT_OF_BOUNDS);       \
    }                                                                           \
    set##arrayType##ArrayElement(pObject, intValue, getOperandStack##type(1));  \
    decrementOperandStackBy(4);                                                 \
    incrementProgramCounter();                                                  \
END_HANDLER


ARRAY_LOAD_HANDLER(I, Int, Int)
ARRAY_LOAD_HANDLER(B, Int, Byte)
ARRAY_LOAD_HANDLER(C, Int, Char)
ARRAY_LOAD_HANDLER(S, Int, Short)
DOUBLE_ARRAY_LOAD_HANDLER(L, Long, Long)

ARRAY_STORE_HANDLER(I, Int, Int)
ARRAY_STORE_HANDLER(B, Int, Byte)
ARRAY_STORE_HANDLER(A, Object, Object)
ARRAY_STORE_HANDLER(C, Int, Char)
ARRAY_STORE_HANDLER(S, Int, Short)
DOUBLE_ARRAY_STORE_HANDLER(L, Long, Long)



#if IMPLEMENTS_FLOAT

ARRAY_LOAD_HANDLER(F, Float, Float)
DOUBLE_ARRAY_LOAD_HANDLER(D, Double, Double)
ARRAY_STORE_HANDLER(F, Float, Float)
DOUBLE_ARRAY_STORE_HANDLER(D, Double, Double)

#endif




#endif








    

