
#ifdef INSIDE_INSTRUCTION_LOOP


START_INSTRUCTION_HANDLER(NOP) 
END_HANDLER

START_INSTRUCTION_HANDLER(GOTO)
    setPC(getPC() + getS2(getPC() + 1));
END_HANDLER

START_INSTRUCTION_HANDLER(GOTO_W)
    setPC(getPC() + getS4(getPC() + 1));
END_HANDLER

/* we must convert the instruction that transfers the return address to the local
 * variable into a RETURN_ADDRESS_STORE instead of an ASTORE, so that ONLY object pointers
 * are stored in the object reference slots in the local variables.  If the ASTORE instruction is
 * not where we expect, we throw an error.  It is highly unlikely it will not be where we expect,
 * it will not occur with the JDK compiler.  If it is not where we expect, we could insert a function
 * that is capable of finding its actual location.
 */
#define alterReturnAddressInstruction(pInstruction) {       \
    switch(*pInstruction) {                                 \
        case ASTORE:                                        \
            *pInstruction = RETURN_ADDRESS_STORE;           \
            break;                                          \
        case ASTORE_0:                                      \
            *pInstruction = RETURN_ADDRESS_STORE_0;         \
            break;                                          \
        case ASTORE_1:                                      \
            *pInstruction = RETURN_ADDRESS_STORE_1;         \
            break;                                          \
        case ASTORE_2:                                      \
            *pInstruction = RETURN_ADDRESS_STORE_2;         \
            break;                                          \
        case ASTORE_3:                                      \
            *pInstruction = RETURN_ADDRESS_STORE_3;         \
            break;                                          \
        case RETURN_ADDRESS_STORE:                          \
        case RETURN_ADDRESS_STORE_0:                        \
        case RETURN_ADDRESS_STORE_1:                        \
        case RETURN_ADDRESS_STORE_2:                        \
        case RETURN_ADDRESS_STORE_3:                        \
            break; /* fall through */                       \
        default:                                            \
            throwError(ERROR_CODE_VIRTUAL_MACHINE);         \
            pNextInstruction = getPC();                     \
            break;                                          \
    }                                                       \
}


START_INSTRUCTION_HANDLER(JSR)
    pushOperandStackRetAddress(getPC() + 3);
#if !USING_GC_STACK_MAPS
    pNextInstruction = getPC() + getS2(getPC() + 1);
    alterReturnAddressInstruction(pNextInstruction);
    setPC(pNextInstruction);
#else
    setPC(getPC() + getS2(getPC() + 1));
#endif
END_HANDLER

START_INSTRUCTION_HANDLER(JSR_W)
    pushOperandStackRetAddress(getPC() + 5);
#if !USING_GC_STACK_MAPS
    pNextInstruction = getPC() + getS4(getPC() + 1);
    alterReturnAddressInstruction(pNextInstruction);
    setPC(pNextInstruction);
#else
    setPC(getPC() + getS4(getPC() + 1));
#endif
END_HANDLER

START_INSTRUCTION_HANDLER(RET)
    setPC(getLocalRetAddress(getPC()[1]));
END_HANDLER

START_INSTRUCTION_HANDLER(TABLESWITCH)
    INT8 padding = 3 - ((getPC() - getCurrentMethodDef()->pCode) % 4);
    INT32 low = getU4(getPC() + (padding + 5));
    INT32 high = getU4(getPC() + (padding + 9));
    intValue = getOperandStackInt(0);
    decrementOperandStack();
    if(((INT32) intValue) < low || ((INT32) intValue) > high) {
        setPC(getPC() + getS4(getPC() + padding + 1));
        instructionBreak();
    }
    setPC(getPC() + getS4(getPC() + padding + 13 + 4*(((INT32) intValue) - low)));
END_HANDLER


START_INSTRUCTION_HANDLER(LOOKUPSWITCH)
    INT32 i;
    INT8 padding = 3 - ((getPC() - getCurrentMethodDef()->pCode) % 4);
    INT32 numPairs = getU4(getPC() + (padding + 5));
    intValue = getOperandStackInt(0);
    decrementOperandStack();
    for(i=0; i<numPairs; i++) {
        UINT32 pairKey = getU4(getPC() + padding + 9 + 8*i);
        if(intValue == pairKey) {
            setPC(getPC() + getS4(getPC() + padding + 13 + 8*i));
            instructionBreak();
        }
        else if(intValue < pairKey) {
            break;
        }
    }
    setPC(getPC() + getS4(getPC() + padding + 1));
END_HANDLER


#define NULL_COMP_INSTRUCTION_HANDLER(IFXX, condition)  \
START_INSTRUCTION_HANDLER(IFXX)                         \
    if(condition) {                                     \
        decrementOperandStack();                        \
        setPC(getPC() + getS2(getPC() + 1));            \
        instructionBreak();                             \
    }                                                   \
    decrementOperandStack();                            \
    incrementProgramCounterBy(3);                       \
END_HANDLER

NULL_COMP_INSTRUCTION_HANDLER(IFNULL, getOperandStackObject(0) == NULL)
NULL_COMP_INSTRUCTION_HANDLER(IFNONNULL, getOperandStackObject(0) != NULL)

#define ZERO_COMP_INSTRUCTION_HANDLER(IFXX, condition)  \
START_INSTRUCTION_HANDLER(IFXX)                         \
    intValue = getOperandStackInt(0);                   \
    decrementOperandStack();                            \
    if(condition) {                                     \
        setPC(getPC() + getS2(getPC() + 1));            \
        instructionBreak();                             \
    }                                                   \
    incrementProgramCounterBy(3);                       \
END_HANDLER

ZERO_COMP_INSTRUCTION_HANDLER(IFEQ, intValue == 0)
ZERO_COMP_INSTRUCTION_HANDLER(IFNE, intValue != 0)
ZERO_COMP_INSTRUCTION_HANDLER(IFLT, intValue < 0)
ZERO_COMP_INSTRUCTION_HANDLER(IFLE, intValue <= 0)
ZERO_COMP_INSTRUCTION_HANDLER(IFGT, intValue > 0)
ZERO_COMP_INSTRUCTION_HANDLER(IFGE, intValue >= 0)

#define COMP_INSTRUCTION_HANDLER(IFXX, condition)       \
START_INSTRUCTION_HANDLER(IFXX)                         \
    if(condition) {                                     \
        decrementOperandStackBy(2);                     \
        setPC(getPC() + getS2(getPC() + 1));            \
        instructionBreak();                             \
    }                                                   \
    decrementOperandStackBy(2);                         \
    incrementProgramCounterBy(3);                       \
END_HANDLER

COMP_INSTRUCTION_HANDLER(IF_ICMPEQ, getOperandStackInt(1) == getOperandStackInt(0))
COMP_INSTRUCTION_HANDLER(IF_ICMPNE, getOperandStackInt(1) != getOperandStackInt(0))
COMP_INSTRUCTION_HANDLER(IF_ICMPLT, getOperandStackInt(1) < getOperandStackInt(0))
COMP_INSTRUCTION_HANDLER(IF_ICMPLE, getOperandStackInt(1) <= getOperandStackInt(0))
COMP_INSTRUCTION_HANDLER(IF_ICMPGT, getOperandStackInt(1) > getOperandStackInt(0))
COMP_INSTRUCTION_HANDLER(IF_ICMPGE, getOperandStackInt(1) >= getOperandStackInt(0))
COMP_INSTRUCTION_HANDLER(IF_ACMPEQ, getOperandStackObject(1) == getOperandStackObject(0))
COMP_INSTRUCTION_HANDLER(IF_ACMPNE, getOperandStackObject(1) != getOperandStackObject(0))

START_INSTRUCTION_HANDLER(LCMP)
    longValue = getOperandStackLong(3);
    longValue2 = getOperandStackLong(1);
    decrementOperandStackBy(3);
    setOperandStackInt(0,
        jlongLessThan(longValue, longValue2) ? -1 :
        jlongGreaterThan(longValue, longValue2) ? 1 : 0);
    incrementProgramCounter();
END_HANDLER

#if IMPLEMENTS_FLOAT

#define FLOAT_COMP_INSTRUCTION_HANDLER(FCMPX, defaultValue)         \
START_INSTRUCTION_HANDLER(FCMPX)                                    \
    setOperandStackInt(1,                                           \
        (getOperandStackFloat(1) < getOperandStackFloat(0)) ? -1 :  \
        (getOperandStackFloat(1) > getOperandStackFloat(0)) ? 1 :   \
        (getOperandStackFloat(1) == getOperandStackFloat(0)) ? 0 :  \
        defaultValue);                                              \
    decrementOperandStack();                                        \
    incrementProgramCounter();                                      \
END_HANDLER

FLOAT_COMP_INSTRUCTION_HANDLER(FCMPL, -1)
FLOAT_COMP_INSTRUCTION_HANDLER(FCMPG, 1)


#define DOUBLE_COMP_INSTRUCTION_HANDLER(DCMPX, defaultValue)            \
START_INSTRUCTION_HANDLER(DCMPX)                                        \
    setOperandStackInt(3,                                               \
        (getOperandStackDouble(3) < getOperandStackDouble(1)) ? -1 :    \
        (getOperandStackDouble(3) > getOperandStackDouble(1)) ? 1 :     \
        (getOperandStackDouble(3) == getOperandStackDouble(1)) ? 0 :    \
        defaultValue);                                                  \
    decrementOperandStackBy(3);                                         \
    incrementProgramCounter();                                          \
END_HANDLER

DOUBLE_COMP_INSTRUCTION_HANDLER(DCMPL, -1)
DOUBLE_COMP_INSTRUCTION_HANDLER(DCMPG, 1)

#endif



#endif








    

