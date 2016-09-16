
#ifdef INSIDE_INSTRUCTION_LOOP



#define ARITHMETHIC_HANDLER(prefix, type, operation, operationSymbol)                               \
START_INSTRUCTION_HANDLER(prefix##operation)                                                        \
    setOperandStack##type(1, getOperandStack##type(1) operationSymbol getOperandStack##type(0));    \
    decrementOperandStack();                                                                        \
    incrementProgramCounter();                                                                      \
END_HANDLER

#define DOUBLE_ARITHMETIC_HANDLER(prefix, type, operation, operationSymbol)                         \
START_INSTRUCTION_HANDLER(prefix##operation)                                                        \
    setOperandStack##type(3, getOperandStack##type(3) operationSymbol getOperandStack##type(1));    \
    decrementOperandStackBy(2);                                                                     \
    incrementProgramCounter();                                                                      \
END_HANDLER
 
ARITHMETHIC_HANDLER(I, Int, ADD, +)
ARITHMETHIC_HANDLER(I, Int, SUB, -)
ARITHMETHIC_HANDLER(I, Int, MUL, *)
ARITHMETHIC_HANDLER(I, Int, AND, &)
ARITHMETHIC_HANDLER(I, Int, OR, |)
ARITHMETHIC_HANDLER(I, Int, XOR, ^)


START_INSTRUCTION_HANDLER(IDIV)
    intValue = getOperandStackInt(0);
    if(intValue == 0) {
        throwExceptionAndBreak(EXCEPTION_CODE_ARITHMETIC);
    }
    setOperandStackInt(1, getOperandStackInt(1) / intValue);
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER


START_INSTRUCTION_HANDLER(IREM)
    intValue = getOperandStackInt(0);
    if(intValue == 0) {
        throwExceptionAndBreak(EXCEPTION_CODE_ARITHMETIC);
    }
    setOperandStackInt(1, getOperandStackInt(1) % intValue);
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(ISHL)
    setOperandStackInt(1, ((INT32) getOperandStackInt(1)) << (0x1F & getOperandStackInt(0)));
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(ISHR)
    setOperandStackInt(1, ((INT32) getOperandStackInt(1)) >> (0x1F & getOperandStackInt(0)));
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(IUSHR)
    setOperandStackInt(1, getOperandStackInt(1) >> (0x1F & getOperandStackInt(0)));
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(IINC)
    getStackFieldInt(pLocalEntry(getPC()[1])) += (INT8) (getPC()[2]);
    incrementProgramCounterBy(3);
END_HANDLER

START_INSTRUCTION_HANDLER(LADD)
    jlongIncrement(getOperandStackLong(3), getOperandStackLong(1));
    decrementOperandStackBy(2);    
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(LSUB)
    jlongDecrement(getOperandStackLong(3), getOperandStackLong(1));
    decrementOperandStackBy(2);    
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(LMUL)
    setOperandStackLong(3, jlongMultiply(getOperandStackLong(3), getOperandStackLong(1)));
    decrementOperandStackBy(2);    
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(LDIV)
    setOperandStackLong(3, jlongDivide(getOperandStackLong(3), getOperandStackLong(1)));
    decrementOperandStackBy(2);    
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(LREM)
    setOperandStackLong(3, jlongRemainder(getOperandStackLong(3), getOperandStackLong(1)));
    decrementOperandStackBy(2);    
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(LSHL)
    setOperandStackLong(3, jlongShiftLeft(getOperandStackLong(2), (getOperandStackInt(0) & 0x3f)));
    decrementOperandStack();    
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(LSHR)
    setOperandStackLong(3, jlongShiftRight(getOperandStackLong(2), (getOperandStackInt(0) & 0x3f)));
    decrementOperandStack();    
    incrementProgramCounter();
END_HANDLER  

START_INSTRUCTION_HANDLER(LUSHR)
    setOperandStackLong(3, jlongUnsignedShiftRight(getOperandStackLong(2), (getOperandStackInt(0) & 0x3f)));
    decrementOperandStack();    
    incrementProgramCounter();
END_HANDLER


START_INSTRUCTION_HANDLER(LNEG)
    jlongNegate(getOperandStackLong(1));
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(LAND)
    jlongBitwiseAnd(getOperandStackLong(3), getOperandStackLong(1));
    decrementOperandStackBy(2);    
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(LOR)
    jlongBitwiseOr(getOperandStackLong(3), getOperandStackLong(1));
    decrementOperandStackBy(2);    
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(LXOR)
    jlongBitwiseEitherOr(getOperandStackLong(3), getOperandStackLong(1));
    decrementOperandStackBy(2);    
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(MATH_QUICK)
    switch(getPC()[1]) {
        case IABS:
            value1 = getOperandStackInt(0);
            if(value1 < 0) {
                setOperandStackInt(0, -value1);
            }
            break;
        case IMIN:
            value1 = getOperandStackInt(1);
            value2 = getOperandStackInt(0);
            if(value1 > value2) {
                setOperandStackInt(1, value2);
            }
            decrementOperandStack();
            break;
        case IMAX:
            value1 = getOperandStackInt(1);
            value2 = getOperandStackInt(0);
            if(value1 < value2) {
                setOperandStackInt(1, value2);
            }
            decrementOperandStack();
            break;

        case LABS:
            longValue = getOperandStackLong(1);
            if(jlongLessThan(longValue, jlongZero)) {
                jlongNegate(longValue);
                setOperandStackLong(1, longValue);
            }
            break;
        case LMIN:
            longValue = getOperandStackLong(3);
            longValue2 = getOperandStackLong(1);
            if(jlongGreaterThan(longValue, longValue2)) {
                setOperandStackLong(3, longValue2);
            }
            break;
        case LMAX:
            longValue = getOperandStackLong(3);
            longValue2 = getOperandStackLong(1);
            if(jlongLessThan(longValue, longValue2)) {
                setOperandStackLong(3, longValue2);
            }
            break;
        default:
            /* should never arrive here */
            break;
    }
    incrementProgramCounterBy(3);
END_HANDLER


#if IMPLEMENTS_FLOAT

ARITHMETHIC_HANDLER(F, Float, ADD, +)
DOUBLE_ARITHMETIC_HANDLER(D, Double, ADD, +)
ARITHMETHIC_HANDLER(F, Float, SUB, -)
DOUBLE_ARITHMETIC_HANDLER(D, Double, SUB, -)
ARITHMETHIC_HANDLER(F, Float, MUL, *)
DOUBLE_ARITHMETIC_HANDLER(D, Double, MUL, *)

START_INSTRUCTION_HANDLER(FDIV)
    setOperandStackFloat(1, getOperandStackFloat(1) / getOperandStackFloat(0));
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(DDIV)
    setOperandStackDouble(3, getOperandStackDouble(3) / getOperandStackDouble(1));
    decrementOperandStackBy(2);
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(FREM)
    setOperandStackFloat(1, jfloatMod(getOperandStackFloat(1), getOperandStackFloat(0)));
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(DREM)
    setOperandStackDouble(3, jdoubleMod(getOperandStackDouble(3), getOperandStackDouble(1)));
    decrementOperandStackBy(2);
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(INEG)
    setOperandStackInt(0,  - (INT32) getOperandStackInt(0));
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(FNEG)
    setOperandStackFloat(0,  - getOperandStackFloat(0));
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(DNEG)
    setOperandStackDouble(1,  - getOperandStackDouble(1));
    incrementProgramCounter();
END_HANDLER

#endif

#endif








    

