
#ifdef INSIDE_INSTRUCTION_LOOP



#define NARROW_INT_CONVERSION(I2X, newType)                     \
START_INSTRUCTION_HANDLER(I2X)                                  \
    setOperandStackInt(0, (newType) getOperandStackInt(0));     \
    incrementProgramCounter();                                  \
END_HANDLER

NARROW_INT_CONVERSION(I2B, UINT8)
NARROW_INT_CONVERSION(I2C, UINT16)
NARROW_INT_CONVERSION(I2S, UINT16)


START_INSTRUCTION_HANDLER(L2I)
    setOperandStackInt(1, jlong2Int(getOperandStackLong(1)));
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(I2L)
    setOperandStackLong(0, jint2Long(longValue, getOperandStackInt(0)));
    incrementOperandStack();
    incrementProgramCounter();
END_HANDLER



#if IMPLEMENTS_FLOAT

START_INSTRUCTION_HANDLER(L2F)
    setOperandStackFloat(1, jlong2Float(getOperandStackLong(1)));
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(L2D)
    setOperandStackDouble(1, jlong2Double(getOperandStackLong(1)));
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(D2L)
    setOperandStackLong(1, jdouble2Long(getOperandStackDouble(1)));
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(F2L)
    setOperandStackLong(0, jfloat2Long(getOperandStackFloat(0)));
    incrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(I2F) 
    setOperandStackFloat(0, (jfloat) getOperandStackInt(0));
    incrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(I2D) 
    setOperandStackDouble(0, (jdouble) getOperandStackInt(0));
    incrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(F2I) 
    setOperandStackInt(0, (UINT32) getOperandStackFloat(0));
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(F2D) 
    setOperandStackDouble(0, (jdouble) getOperandStackFloat(0));
    incrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(D2I)
    setOperandStackInt(1, (UINT32) getOperandStackDouble(1));
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

START_INSTRUCTION_HANDLER(D2F)
    setOperandStackFloat(1, (jfloat) getOperandStackDouble(1));
    decrementOperandStack();
    incrementProgramCounter();
END_HANDLER

#endif



#endif








    

