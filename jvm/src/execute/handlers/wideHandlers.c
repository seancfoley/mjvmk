
#ifdef INSIDE_INSTRUCTION_LOOP



#define WIDE_LOAD_HANDLER(prefix, type)                                 \
START_WIDE_INSTRUCTION_HANDLER(prefix##LOAD)                                 \
    pushOperandStack##type(getLocal##type##(getU2(getPC() + 1)));       \
    incrementProgramCounterBy(3);                                       \
END_HANDLER

#define WIDE_STORE_HANDLER(prefix, type)                                \
START_WIDE_INSTRUCTION_HANDLER(prefix##STORE)                                \
    popOperandStack##type##IntoLocal(pLocalEntry(getU2(getPC() + 1)));  \
    incrementProgramCounterBy(3);                                       \
END_HANDLER


WIDE_LOAD_HANDLER(A, Object)
WIDE_LOAD_HANDLER(I, Int)
WIDE_LOAD_HANDLER(L, Long)

WIDE_STORE_HANDLER(A, Object)
WIDE_STORE_HANDLER(I, Int)
WIDE_STORE_HANDLER(L, Long)
WIDE_STORE_HANDLER(RETURN_ADDRESS_, RetAddress)


START_WIDE_INSTRUCTION_HANDLER(IINC)
    getStackFieldInt(pLocalEntry(getU2(getPC() + 1))) += getS2(getPC() + 3);
    incrementProgramCounterBy(5);
END_HANDLER

START_WIDE_INSTRUCTION_HANDLER(RET)
    setPC(getLocalRetAddress(getU2(getPC() + 1)));
END_HANDLER


#if IMPLEMENTS_FLOAT

WIDE_LOAD_HANDLER(F, Float)
WIDE_LOAD_HANDLER(D, Double)
WIDE_STORE_HANDLER(F, Float)
WIDE_STORE_HANDLER(D, Double)

#endif




#endif








    

