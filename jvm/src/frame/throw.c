
#include "interpret/instructions.h"
#include "javaThread/javaThread.h"
#include "object/instantiate.h"
#include "string/stringPool.h"
#include "thread/schedule.h"
#include "log/logItem.h"
#include "throw.h"



static BOOLEAN foundHandler(OBJECT_INSTANCE pThrowableObject, THROWABLE_HANDLER_LIST pHandlerList, 
                     UINT16 framePCIndex, UINT16 *pNewProgramCounterIndex) 
{
    THROWABLE_HANDLER pHandler;
    UINT16 i;
    COMMON_CLASS_DEF pClassDef = pThrowableObject->header.pClassDef;
    
    for(i=0; i<pHandlerList->length; i++) {
        pHandler = pHandlerList->ppHandlers[i];

        /* note: the end handler index is not inclusive, but the program counter was incremented when the 
         * frame was pushed, so the test below includes the end index for the handler
         */
        if(framePCIndex >= pHandler->startPCIndex && framePCIndex <= pHandler->endPCIndex) {
            if(pHandler->catchKey.namePackageKey == 0 || pHandler->catchKey.namePackageKey == pClassDef->key.namePackageKey) {
                *pNewProgramCounterIndex = pHandler->handlerPCIndex;
                return TRUE;
            }
            while(TRUE) {
                pClassDef = &pClassDef->pSuperClass->header;
                if(pClassDef == NULL) {
                    break;
                }
                if(pHandler->catchKey.namePackageKey == pClassDef->key.namePackageKey) {
                    *pNewProgramCounterIndex = pHandler->handlerPCIndex;
                    return TRUE;
                }
            }
            pClassDef = pThrowableObject->header.pClassDef;
        }
    }
    return FALSE;
}

#if PREEMPTIVE_THREADS

monitorStruct stackTraceMonitor = {0, NULL, NULL, NULL};
MONITOR pStackTraceMonitor = &stackTraceMonitor;

#endif

/* Note that we will ALWAYS find a handler due to the use of the catch-all method at the base of every stack frame */
void findNewFrame(OBJECT_INSTANCE *ppThrowableObject, RESET_REGISTERS pRegisters)
{
    METHOD_DEF pMethodDef;
    THROWABLE_HANDLER_LIST pHandlerList;
    UINT16 programCounterIndex;
#if CREATE_THROWABLE_STACK_TRACES
    OBJECT_INSTANCE pStackTrace;
#endif
    throwRegisterStruct originalRegisters = *pRegisters;
    
    do {
        pMethodDef = getFrameMethod(pRegisters->fp);
        pHandlerList = pMethodDef->throwableHandlers;
        programCounterIndex = pRegisters->pc - pMethodDef->pCode;
        if(pHandlerList != NULL && 
            foundHandler(*ppThrowableObject, pHandlerList, programCounterIndex, &programCounterIndex)) {

            pRegisters->pc = pMethodDef->pCode + programCounterIndex;
            if(pRegisters->pc[0] == FIRST_OPCODE) {

                /* we traced down to the bottom of the stack */
                THROWABLE_INSTANCE pThrowable = getThrowableInstanceFromObjectInstance(*ppThrowableObject);
                NamePackage np = pThrowable->header.pClassDef->key;
                UINT16 nameLength;
                UINT16 pkgLength;
                char *pkg = getPackageNameStringByKey(np.np.packageKey, &pkgLength);
                char *name = getUTF8StringByKey(np.np.nameKey, &nameLength);
                
                enterStackTraceMonitor();
                psPrintErr("Unhandled exception: ");
                psPrintNErr(pkg, pkgLength);
                if(pkgLength) {
                    psPutCharErr('/');
                }
                psPrintNErr(name, nameLength);
                psPutCharErr('\n');
#if CREATE_THROWABLE_STACK_TRACES
                pStackTrace = extractObjectFieldObject(pThrowable->stackTrace);
                if(pStackTrace != NULL) {
                    printStackTrace(pStackTrace);
                }
                else {
                    psPrintErr("Exception stack trace unavailable, printing current stack trace\n");
                    printCurrentStackTrace(&originalRegisters);
                }
#else
                psPrintErr("Exception stack trace unavailable, printing current stack trace\n");
                printCurrentStackTrace(&originalRegisters);
#endif
                exitStackTraceMonitor();
            }
            return;
        }
        if(isSynchronizedMethod(pMethodDef)) {
            if(monitorNotOwnedByCurrentJavaThread(getMonitorFromFrame(pRegisters->fp))) {

                /* Note that it's compulsary that the IllegalMonitorStateException object be
                 * created at startup.  The reason for this is that we may have been originally 
                 * trying to throw an OutOfMemoryError, so we certainly cannot be attempting to
                 * create any new objects at this time.
                 */
                *ppThrowableObject = pJavaLangIllegalMonitorStateExceptionInstance;
            }
            monitorExit(getMonitorFromFrame(pRegisters->fp));
        }
        pRegisters->pc = getFramePreviousPC(pRegisters->fp);
        pRegisters->fp = getFramePreviousFP(pRegisters->fp);        
    } while(TRUE);
    return;
}


static void printStackTraceFrame(METHOD_DEF pMethodDef, COMMON_CLASS_DEF pClassDef)
{
    char *pkg;
    UINT16 pkgLength;
    char *className;
    UINT16 classNameLength;
    char *methodName;
    UINT16 methodNameLength;
    char *methodType;
    UINT16 methodTypeLength;

    pkg = getPackageNameStringByKey(getPackageKey(pClassDef), &pkgLength);
    className = getUTF8StringByKey(getNameKey(pClassDef), &classNameLength);
    methodName = getUTF8StringByKey(getMethodNameKey(pMethodDef), &methodNameLength);
    methodType = getMethodTypeStringByKey(getMethodTypeKey(pMethodDef), &methodTypeLength);
    psPrintErr(" at ");
    psPrintNErr(pkg, pkgLength);
    psPutCharErr('/');
    psPrintNErr(className, classNameLength);
    psPutCharErr('.');
    psPrintNErr(methodName, methodNameLength);
    psPutCharErr(' ');
    psPrintNErr(methodType, methodTypeLength);
    psPrintErr("\n");
    return;
}



void printCurrentStackTrace(CODE_REGISTERS pRegisters)
{
    METHOD_DEF pMethodDef;
    LOADED_CLASS_DEF pClassDef;
    INSTRUCTION pc = pRegisters->pc;
     
    do {
        pMethodDef = getFrameMethod(pRegisters->fp);
        pClassDef = pMethodDef->base.pOwningClass;
        printStackTraceFrame(pMethodDef, (COMMON_CLASS_DEF) pClassDef);
        pc = getFramePreviousPC(pRegisters->fp);

        /* check for bottom of stack */
        switch(*pc) {
            case FIRST_OPCODE:
            case CLINIT_OPCODE:
            case INIT_OPCODE:
                return;
        }
        pRegisters->fp = getFramePreviousFP(pRegisters->fp);
   } while(TRUE);

    /* should never reach here */
}



#if CREATE_THROWABLE_STACK_TRACES


/* stack traces are created as a linked list of stack trace instances, each instance containing 
 * an array of stack trace frame structures and a reference to the next instance.
 */
RETURN_CODE createStackTrace(OBJECT_INSTANCE pThrowableObject, CODE_REGISTERS pRegisters)
{
    THROWABLE_INSTANCE pThrowable = getThrowableInstanceFromObjectInstance(pThrowableObject);
    METHOD_DEF pMethodDef;
    LOADED_CLASS_DEF pClassDef;
    RETURN_CODE ret;
    STACK_TRACE_INSTANCE pCurrentStackTraceInstance = NULL;
    STACK_TRACE_CLASS_ARRAY pCurrentClassArrayInstance;
    STACK_TRACE_METHOD_ARRAY pCurrentMethodArrayInstance;
    UINT16 arrayIndex = 0;
    UINT32 currentBlockIndex;
    INSTRUCTION pc = pRegisters->pc;
    OBJECT_INSTANCE *ppStackTraceBeginning = extractObjectFieldObjectPtr(pThrowable->stackTrace);
    OBJECT_INSTANCE *ppInstance = ppStackTraceBeginning;
     
    do {
        ret = instantiateCollectibleObject(pComMjvmkStackTrace, ppInstance);
        if(ret != SUCCESS) {
            return ret;
        }
        pCurrentStackTraceInstance = getStackTraceInstanceFromObjectInstance(*ppInstance);
        ret = instantiateCollectiblePrimitiveArray(pIntegerArrayClass, STACK_TRACE_INCREMENTS, (ARRAY_INSTANCE *) extractObjectFieldObjectPtr(pCurrentStackTraceInstance->stackTraceMethodArray));
        if(ret != SUCCESS) {
            return ret;
        }
        ret = instantiateCollectiblePrimitiveArray(pShortArrayClass, STACK_TRACE_INCREMENTS, (ARRAY_INSTANCE *) extractObjectFieldObjectPtr(pCurrentStackTraceInstance->stackTraceClassArray));
        if(ret != SUCCESS) {
            return ret;
        }

        pCurrentClassArrayInstance = (STACK_TRACE_CLASS_ARRAY) extractObjectFieldObject(pCurrentStackTraceInstance->stackTraceClassArray);
        pCurrentMethodArrayInstance = (STACK_TRACE_METHOD_ARRAY) extractObjectFieldObject(pCurrentStackTraceInstance->stackTraceMethodArray);
        
        for(currentBlockIndex=0; currentBlockIndex < STACK_TRACE_INCREMENTS; currentBlockIndex++) {

            pMethodDef = getFrameMethod(pRegisters->fp);
            pClassDef = pMethodDef->base.pOwningClass;            
            pCurrentClassArrayInstance->values[currentBlockIndex] = getClassTableIndexFromClass(pClassDef);
            pCurrentMethodArrayInstance->values[currentBlockIndex] = pMethodDef->base.methodIndex;
            if(isStaticMethod(pMethodDef)) {
                pCurrentMethodArrayInstance->values[currentBlockIndex] |= (1 << 16);
            }

            pc = getFramePreviousPC(pRegisters->fp);

            /* check for bottom of stack */
            switch(*pc) {
                case FIRST_OPCODE:
                case CLINIT_OPCODE:
                case INIT_OPCODE:
                    return SUCCESS;
            }
            pRegisters->fp = getFramePreviousFP(pRegisters->fp);
        }
        ppInstance = extractObjectFieldObjectPtr(pCurrentStackTraceInstance->nextStackTraceInstance);
    } while(TRUE);

    /* should never reach here */
    return SUCCESS;
}

void printStackTrace(OBJECT_INSTANCE pStackTraceObject)
{
    STACK_TRACE_INSTANCE pStackTrace = getStackTraceInstanceFromObjectInstance(pStackTraceObject);
    STACK_TRACE_CLASS_ARRAY pClassArray;
    STACK_TRACE_METHOD_ARRAY pMethodArray;
    METHOD_DEF pMethodDef;
    COMMON_CLASS_DEF pClassDef;
    UINT16 methodIndex;
    UINT16 classIndex;
    BOOLEAN isStaticMethod;
    UINT16 index = 0;

    pClassArray = (STACK_TRACE_CLASS_ARRAY) extractObjectFieldObject(pStackTrace->stackTraceClassArray);
    pMethodArray = (STACK_TRACE_METHOD_ARRAY) extractObjectFieldObject(pStackTrace->stackTraceMethodArray);
    while(TRUE) {
        classIndex = pClassArray->values[index];
        if(classIndex == 0) {
            break;
        }
        methodIndex = pMethodArray->values[index] & 0xffff;
        isStaticMethod = pMethodArray->values[index] >> 16;
        pClassDef = getClassInTableByIndex(classIndex)->pRepresentativeClassDef;
        pMethodDef = isStaticMethod ? getStaticMethod(pClassDef, methodIndex) : getInstanceMethod(pClassDef, methodIndex);
        printStackTraceFrame(pMethodDef, pClassDef);
        index++;
        if(index == STACK_TRACE_INCREMENTS) {
            pStackTraceObject = extractObjectFieldObject(pStackTrace->nextStackTraceInstance);
            if(pStackTraceObject == NULL) {
                break;
            }
            pStackTrace = getStackTraceInstanceFromObjectInstance(pStackTraceObject);
            pClassArray = (STACK_TRACE_CLASS_ARRAY) extractObjectFieldObject(pStackTrace->stackTraceClassArray);
            pMethodArray = (STACK_TRACE_METHOD_ARRAY) extractObjectFieldObject(pStackTrace->stackTraceMethodArray);
            index = 0;
        }
    }
    return;
}

#endif



