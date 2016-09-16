
#ifdef INSIDE_INSTRUCTION_LOOP



START_INSTRUCTION_HANDLER(POP)
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(POP2)
    decrementOperandStackBy(2);
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(DUP)
    incrementOperandStack();
    setOperandStackEntry(0, getOperandStackEntry(1));
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(DUP_X1)
    fieldValue = getOperandStackEntry(1);
    fieldValue2 = getOperandStackEntry(0);
    setOperandStackEntry(1, fieldValue2);
    setOperandStackEntry(0, fieldValue);
    pushOperandStackEntry(fieldValue2);
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(DUP_X2)
    fieldValue = getOperandStackEntry(2);
    fieldValue2 = getOperandStackEntry(1);
    fieldValue3 = getOperandStackEntry(0);
    setOperandStackEntry(2, fieldValue3);
    setOperandStackEntry(1, fieldValue);
    setOperandStackEntry(0, fieldValue2);
    pushOperandStackEntry(fieldValue3);
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(DUP2)
    incrementOperandStackBy(2);
    setOperandStackEntry(1, getOperandStackEntry(3));
    setOperandStackEntry(0, getOperandStackEntry(2));
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(DUP2_X1)
    fieldValue = getOperandStackEntry(2);
    fieldValue2 = getOperandStackEntry(1);
    fieldValue3 = getOperandStackEntry(0);
    setOperandStackEntry(2, fieldValue2);
    setOperandStackEntry(1, fieldValue3);
    setOperandStackEntry(0, fieldValue);
    pushOperandStackEntry(fieldValue2);
    pushOperandStackEntry(fieldValue3);
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(DUP2_X2)
    fieldValue = getOperandStackEntry(3);
    fieldValue2 = getOperandStackEntry(2);
    fieldValue3 = getOperandStackEntry(1);
    fieldValue4 = getOperandStackEntry(0);
    setOperandStackEntry(3, fieldValue3);
    setOperandStackEntry(2, fieldValue4);
    setOperandStackEntry(1, fieldValue);
    setOperandStackEntry(0, fieldValue2);
    pushOperandStackEntry(fieldValue3);
    pushOperandStackEntry(fieldValue4);
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(SWAP)
    fieldValue = getOperandStackEntry(1);
    setOperandStackEntry(1, getOperandStackEntry(0));
    setOperandStackEntry(0, fieldValue);
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(BIPUSH)
    pushOperandStackInt(getPC()[1]);
    incrementProgramCounterBy(2);
END_HANDLER

START_INSTRUCTION_HANDLER(SIPUSH)
    pushOperandStackInt(getU2(getPC() + 1));
    incrementProgramCounterBy(3);
END_HANDLER


#define CONST_HANDLER(prefix, suffix, type, value)          \
START_INSTRUCTION_HANDLER(prefix##CONST_##suffix)           \
    pushOperandStack##type(value);                          \
    incrementProgramCounter();                              \
END_HANDLER

CONST_HANDLER(A, NULL, Object, NULL)
CONST_HANDLER(I, M1, Int, -1)
CONST_HANDLER(I, 0, Int, 0)
CONST_HANDLER(I, 1, Int, 1)
CONST_HANDLER(I, 2, Int, 2)
CONST_HANDLER(I, 3, Int, 3)
CONST_HANDLER(I, 4, Int, 4)
CONST_HANDLER(I, 5, Int, 5)
CONST_HANDLER(L, 0, Long, jlongZero)
CONST_HANDLER(L, 1, Long, jlongOne)


#if IMPLEMENTS_FLOAT

CONST_HANDLER(F, 0, Float, 0.0)
CONST_HANDLER(F, 1, Float, 1.0)
CONST_HANDLER(F, 2, Float, 2.0)
CONST_HANDLER(D, 0, Double, 0.0)
CONST_HANDLER(D, 1, Double, 1.0)

#endif



#endif








    

