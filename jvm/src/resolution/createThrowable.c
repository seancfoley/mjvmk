
#include "object/instantiate.h"
#include "string/stringKeys.h"
#include "javaThread/javaThread.h"
#include "resolution/resolve/resolve.h"
#include "createThrowable.h"



static RETURN_CODE createThrowableObject(NamePackage key, OBJECT_INSTANCE *ppInstance, CODE_REGISTERS pStackTraceRegisters)
{
    CLASS_INSTANCE pClassInstance;
    RETURN_CODE ret;
    
    ret = obtainClassRef(key, &pClassInstance);
    if(ret != SUCCESS) {
        return ret;
    }

    ret = instantiateCollectibleObject(pClassInstance, ppInstance);
    if(ret != SUCCESS) {
        return ret;
    }
    
#if CREATE_THROWABLE_STACK_TRACES
    {
        ret = createStackTrace(*ppInstance, pStackTraceRegisters);
        if(ret != SUCCESS) {
            return ret;
        }
    }
#endif
    return SUCCESS;
}

/* the object will be place in the pObjectToThrow member of the thread structure as well as being returned by the function */
OBJECT_INSTANCE getThrowableErrorObject(JAVA_THREAD pThread, RETURN_CODE ret, INSTRUCTION stackTracePC, FRAME stackTraceFP)
{
    NamePackage key;
    UINT8 counter = 0; /* prevents an infinite loop from occurring */
    throwRegisterStruct reg;

    reg.pc = stackTracePC;
    reg.fp = stackTraceFP;

    while(TRUE) {
        switch(ret) {
            case ERROR_CODE_OUT_OF_MEMORY:
                pThread->pObjectToThrow = pJavaLangOutOfMemoryErrorInstance;
                return pJavaLangOutOfMemoryErrorInstance;
            
            case ERROR_CODE_STACK_OVERFLOW:
                pThread->pObjectToThrow = pJavaLangStackOverflowErrorInstance;
                return pJavaLangStackOverflowErrorInstance;
            
            case ERROR_CODE_VIRTUAL_MACHINE:
            case ERROR_CODE_CLASS_TABLE_FULL:
                key = javaLangVirtualMachineErrorKey;
                break;

            case ERROR_CODE_EXCEPTION_IN_INITIALIZER:
                key = javaLangExceptionInInitializerErrorKey;
                break;

            case ERROR_CODE_ABSTRACT_METHOD:
            case ERROR_CODE_CIRCULARITY:
            case ERROR_CODE_ILLEGAL_ACCESS:
            case ERROR_CODE_INCOMPATIBLE_CLASS_CHANGE:
            case ERROR_CODE_INSTANTIATION:
            case ERROR_CODE_NO_SUCH_FIELD:
            case ERROR_CODE_NO_SUCH_METHOD:
            case ERROR_CODE_UNSATISFIED_LINK:
            case ERROR_CODE_UNSUPPORTED_CLASS_VERSION:
            case ERROR_CODE_VERIFY:
            case ERROR_CODE_CLASS_FORMAT:
            case ERROR_CODE_INVALID_FLAGS:
            case ERROR_CODE_INVALID_SUPER_CLASS:
            case ERROR_CODE_INVALID_SUPER_INTERFACE:
            case ERROR_CODE_INVALID_FILE_SIZE:
            case ERROR_CODE_FILE_READ_ERROR:
            case ERROR_CODE_INCOMPLETE_CONSTANT_POOL_ENTRY:
            case ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY:
            case ERROR_CODE_INVALID_MEMBER_REF:
            case ERROR_CODE_DUPLICATE_METHOD:
            case ERROR_CODE_DUPLICATE_FIELD:
            case ERROR_CODE_OVERRIDDEN_FINAL_METHOD:
            case ERROR_CODE_NO_METHOD_CODE:
            case ERROR_CODE_INVALID_UTF8_STRING:
            case ERROR_CODE_INVALID_NAME:
            case ERROR_CODE_INVALID_MAGIC_NUMBER:
            case ERROR_CODE_INVALID_SELF_REFERENCE:
            case ERROR_CODE_INVALID_STATIC_CONSTANT:
            case ERROR_CODE_INVALID_ATTRIBUTE:
            case ERROR_CODE_INVALID_ARRAY_TYPE:
            case ERROR_CODE_INVALID_FIELD_TYPE:
            case ERROR_CODE_INVALID_METHOD_TYPE:

                key = javaLangLinkageErrorKey;
                break;
            
            case ERROR_CODE_NO_CLASS_DEF_FOUND:
                key = javaLangNoClassDefFoundErrorKey;
                break;

            default:
                pThread->pObjectToThrow = pJavaLangErrorInstance;
                return pJavaLangErrorInstance;
        }
        ret = createThrowableObject(key, &pThread->pObjectToThrow, &reg);
        if(ret == SUCCESS) {
            break;
        }
        else if(counter >= 2) {
            pThread->pObjectToThrow = pJavaLangErrorInstance;
            return pJavaLangErrorInstance;
        }
        counter++;
    }
    return pThread->pObjectToThrow;
}



OBJECT_INSTANCE getThrowableExceptionObject(JAVA_THREAD pThread, EXCEPTION_CODE exceptionCode, INSTRUCTION stackTracePC, FRAME stackTraceFP)
{
    NamePackage key;
    RETURN_CODE ret;
    throwRegisterStruct reg;

    reg.pc = stackTracePC;
    reg.fp = stackTraceFP;

    switch(exceptionCode) {
        case EXCEPTION_CODE_IO:
            key = javaIOExceptionKey;
            break;

        case EXCEPTION_CODE_NULL_POINTER:
            key = javaLangNullPointerExceptionKey;
            break;

        case EXCEPTION_CODE_CLASS_CAST:
            key = javaLangClassCastExceptionKey;
            break;

        case EXCEPTION_CODE_ARRAY_INDEX_OUT_OF_BOUNDS:
            key = javaLangArrayIndexOutOfBoundsExceptionKey;
            break;

        case EXCEPTION_CODE_ARITHMETIC:
            key = javaLangArithmeticExceptionKey;
            break;

        case EXCEPTION_CODE_NEGATIVE_ARRAY_SIZE:
            key = javaLangNegativeArraySizeExceptionKey;
            break;

        case EXCEPTION_CODE_INTERRUPTED:
            key = javaLangInterruptedExceptionKey;
            break;

        case EXCEPTION_CODE_INSTANTIATION:
            key = javaLangInstantiationExceptionKey;
            break;

        case EXCEPTION_CODE_ILLEGAL_ACCESS:
            key = javaLangIllegalAccessExceptionKey;
            break;

        case EXCEPTION_CODE_ARRAY_STORE:
            key = javaLangArrayStoreExceptionKey;
            break;

        case EXCEPTION_CODE_ILLEGAL_ARGUMENT:
            key = javaLangIllegalArgumentExceptionKey;
            break;

        case EXCEPTION_CODE_ILLEGAL_MONITOR_STATE:
            key = javaLangIllegalMonitorStateExceptionKey;
            break;

        case EXCEPTION_CODE_ILLEGAL_THREAD_STATE:
            key = javaLangIllegalThreadStateExceptionKey;
            break;

        case EXCEPTION_CODE_STRING_INDEX_OUT_OF_BOUNDS:
            key = javaLangStringIndexOutOfBoundsExceptionKey;
            break;


        default:
            /* should never arrive here */
            return getThrowableErrorObject(pThread, ERROR_CODE_VIRTUAL_MACHINE, stackTracePC, stackTraceFP);
    }
    ret = createThrowableObject(key, &pThread->pObjectToThrow, &reg);
    if(ret != SUCCESS) {
        return getThrowableErrorObject(pThread, ret, stackTracePC, stackTraceFP);
    }
    return pThread->pObjectToThrow;
}
