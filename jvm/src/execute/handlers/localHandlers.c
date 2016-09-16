
#ifdef INSIDE_INSTRUCTION_LOOP

#define LOAD_HANDLER(prefix, type)                              \
START_INSTRUCTION_HANDLER(prefix##LOAD)                         \
    pushOperandStack##type(getLocal##type##(getPC()[1]));       \
    incrementProgramCounterBy(2);                               \
END_HANDLER

#define LOAD_X_HANDLER(prefix, type, index)                     \
START_INSTRUCTION_HANDLER(prefix##LOAD_##index)                 \
    pushOperandStack##type(getLocal##type##(index));            \
    incrementProgramCounter();                                  \
END_HANDLER

#define STORE_HANDLER(prefix, type)                             \
START_INSTRUCTION_HANDLER(prefix##STORE)                        \
    popOperandStack##type##IntoLocal(pLocalEntry(getPC()[1]));  \
    incrementProgramCounterBy(2);                               \
END_HANDLER

#define STORE_X_HANDLER(prefix, type, index)                    \
START_INSTRUCTION_HANDLER(prefix##STORE_##index)                \
    popOperandStack##type##IntoLocal(pLocalEntry(index));       \
    incrementProgramCounter();                                  \
END_HANDLER


LOAD_HANDLER(I, Int)
LOAD_HANDLER(L, Long)
LOAD_HANDLER(A, Object)

LOAD_X_HANDLER(I, Int, 0)
LOAD_X_HANDLER(I, Int, 1)
LOAD_X_HANDLER(I, Int, 2)
LOAD_X_HANDLER(I, Int, 3)

LOAD_X_HANDLER(L, Long, 0)
LOAD_X_HANDLER(L, Long, 1)
LOAD_X_HANDLER(L, Long, 2)
LOAD_X_HANDLER(L, Long, 3)

LOAD_X_HANDLER(A, Object, 0)
LOAD_X_HANDLER(A, Object, 1)
LOAD_X_HANDLER(A, Object, 2)
LOAD_X_HANDLER(A, Object, 3)


STORE_HANDLER(I, Int)
STORE_HANDLER(L, Long)
STORE_HANDLER(A, Object)
STORE_HANDLER(RETURN_ADDRESS_, RetAddress)

STORE_X_HANDLER(I, Int, 0)
STORE_X_HANDLER(I, Int, 1)
STORE_X_HANDLER(I, Int, 2)
STORE_X_HANDLER(I, Int, 3)

STORE_X_HANDLER(L, Long, 0)
STORE_X_HANDLER(L, Long, 1)
STORE_X_HANDLER(L, Long, 2)
STORE_X_HANDLER(L, Long, 3)

STORE_X_HANDLER(A, Object, 0)
STORE_X_HANDLER(A, Object, 1)
STORE_X_HANDLER(A, Object, 2)
STORE_X_HANDLER(A, Object, 3)

STORE_X_HANDLER(RETURN_ADDRESS_, RetAddress, 0)
STORE_X_HANDLER(RETURN_ADDRESS_, RetAddress, 1)
STORE_X_HANDLER(RETURN_ADDRESS_, RetAddress, 2)
STORE_X_HANDLER(RETURN_ADDRESS_, RetAddress, 3)

#if IMPLEMENTS_FLOAT

LOAD_HANDLER(F, Float)
LOAD_HANDLER(D, Double)

LOAD_X_HANDLER(F, Float, 0)
LOAD_X_HANDLER(F, Float, 1)
LOAD_X_HANDLER(F, Float, 2)
LOAD_X_HANDLER(F, Float, 3)

LOAD_X_HANDLER(D, Double, 0)
LOAD_X_HANDLER(D, Double, 1)
LOAD_X_HANDLER(D, Double, 2)
LOAD_X_HANDLER(D, Double, 3)

STORE_HANDLER(F, Float)
STORE_HANDLER(D, Double)

STORE_X_HANDLER(F, Float, 0)
STORE_X_HANDLER(F, Float, 1)
STORE_X_HANDLER(F, Float, 2)
STORE_X_HANDLER(F, Float, 3)

STORE_X_HANDLER(D, Double, 0)
STORE_X_HANDLER(D, Double, 1)
STORE_X_HANDLER(D, Double, 2)
STORE_X_HANDLER(D, Double, 3)

#endif



#endif








    

