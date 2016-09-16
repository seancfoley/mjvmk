
#include "memory/memoryAlloc.h"
#include "memory/garbageCollect.h"
#include "interpret/instructions.h"
#include "interpret/returnCodes.h"
#include "object/instantiate.h"
#include "string/javaString.h"
#include "string/stringKeys.h"
#include "thread/schedule.h"
#include "class/typeCheck.h"
#include "resolution/file/fileAccess.h"
#include "resolution/resolve/resolve.h"
#include "resolution/resolve/lookup.h"
#include "resolution/resource.h"
#include "execute/execute.h"
#include "execute/javaRegisterSwitch.h"
#include "execute/frameRegisters.h"
#include "natives.h"


/* the first local will contain the new registers, the second will point
 * to the location on the stack for the thrown object.
 */
#define THROWING_LOCALS throwRegisterStruct throwRegisters; JSTACK_FIELD pFieldValue;

#if PREEMPTIVE_THREADS
#define checkForInterruptedExceptionNative()                            \
    if(currentJavaThreadIsInterrupted()) {                              \
        clearCurrentJavaThreadInterruptedFlag();                        \
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_INTERRUPTED);  \
    }
#else
#define checkForInterruptedExceptionNative()
#endif
    


void javaLangRuntimeExit()
{
    //TODO PROCESS MODEL: kill and clean up the current process - this is essentially just removing the current thread
    //and all associated threads in the switch code
    induceJavaThreadSwitch();
}

void javaLangRuntimeGC()
{
    garbageCollect();
    decrementOperandStackNative();
    incrementNativeProgramCounterVirtual();
}

void javaLangRuntimeFreeMemory()
{
    jlong res;

    setJlong(res, 0, getTotalAvailableMemory());
    setOperandStackLongNative(0, res);
    incrementOperandStackNative();
    incrementNativeProgramCounterVirtual();
}

void javaLangRuntimeTotalMemory()
{
    jlong res;

    setJlong(res, 0, getTotalMemory());
    setOperandStackLongNative(0, res);
    incrementOperandStackNative();
    incrementNativeProgramCounterVirtual();
}


void javaLangSystemGetProperty()
{
    THROWING_LOCALS
    OBJECT_INSTANCE pObjectKey = getOperandStackObjectNative(0);
    STRING_INSTANCE pKey;
    char *key;
    UINT32 keyLength;
    RETURN_CODE ret;
    char line[MAX_PROPERTY_LINE_LEN];
    char *property;
    UINT16 propertyLength;

    if(pObjectKey == NULL) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_NULL_POINTER);
    }
    pKey = getStringInstanceFromObjectInstance(pObjectKey);
    keyLength = extractPrimitiveFieldInt(pKey->length);
    if(keyLength == 0) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_ARGUMENT);
    }
    key = memoryAlloc(keyLength);
    if(key == NULL) {
        throwErrorFromNativeAndReturn(ERROR_CODE_OUT_OF_MEMORY);
    }
    getUTF8StringFromStringInstance(pKey, key);
    ret = getProperty(key, &propertyLength, line, &property);
    memoryFree(key);
    if(ret != SUCCESS) {
        throwErrorFromNativeAndReturn(ret);
    }
    if(property == NULL) {
        setOperandStackObjectNative(0, NULL);
    }
    else {
        ret = instantiateString(property, propertyLength, getOperandStackObjectPointerNative(0));
        if(ret != SUCCESS) {
            throwErrorFromNativeAndReturn(ret);
        }
    }       
    incrementNativeProgramCounterRegular();
    return;
}

    

void javaLangSystemCurrentTimeMillis()
{
    jlong time;
    timeValueStruct timeValue;

    getCurrentTime(&timeValue);
    getTime(time, &timeValue);
    pushOperandStackLongNative(time);
    incrementNativeProgramCounterRegular();
}

void javaLangSystemArrayCopy()
{
    ARRAY_INSTANCE pSource = (ARRAY_INSTANCE) getOperandStackObjectNative(4);
    INT32 srcPosition = getOperandStackIntNative(3);
    ARRAY_INSTANCE pDest = (ARRAY_INSTANCE) getOperandStackObjectNative(2);
    INT32 dstPosition = getOperandStackIntNative(1);
    INT32 length = getOperandStackIntNative(0);
    ARRAY_CLASS_DEF pSourceClassDef;
    ARRAY_CLASS_DEF pDestClassDef;
    THROWING_LOCALS

    if(pSource == NULL || pDest == NULL) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_NULL_POINTER);
    }
    pSourceClassDef = (ARRAY_CLASS_DEF) pSource->header.header.pClassDef;
    pDestClassDef = (ARRAY_CLASS_DEF) pDest->header.header.pClassDef;

    /* first ensure we do indeed have array objects */
    if(!(isArrayClass(pSourceClassDef) && isArrayClass(pDestClassDef))) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ARRAY_STORE);
    }

    /* check the indices */
    if (length < 0 || srcPosition < 0 || dstPosition < 0
          || srcPosition + length > (INT32) pSource->header.arrayLength
          || dstPosition + length > (INT32) pDest->header.arrayLength) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ARRAY_INDEX_OUT_OF_BOUNDS);
    }

    /* make sure the array types are compatible */
    if(isPrimitiveArrayClass(pSourceClassDef)) {
        if(!isPrimitiveArrayClass(pDestClassDef)) {
            throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ARRAY_STORE);
        }
        else {
            ARRAY_TYPE sourceType = getPrimitiveArrayType(pSourceClassDef);
            ARRAY_TYPE destType = getPrimitiveArrayType(pDestClassDef);

            if(sourceType != destType) {
                throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ARRAY_STORE);
            }
            
            switch(sourceType) {

                case BOOLEAN_ARRAY_TYPE:
                    memoryMove(((BOOLEAN_ARRAY_INSTANCE) pDest)->values + dstPosition,
                        ((BOOLEAN_ARRAY_INSTANCE) pSource)->values + srcPosition,
                        length * pSourceClassDef->elementByteSize);
                    break;
                case CHAR_ARRAY_TYPE:
                    memoryMove(((CHAR_ARRAY_INSTANCE) pDest)->values + dstPosition,
                        ((CHAR_ARRAY_INSTANCE) pSource)->values + srcPosition,
                        length * pSourceClassDef->elementByteSize);
                    break;
                case INT_ARRAY_TYPE:
                    memoryMove(((INT_ARRAY_INSTANCE) pDest)->values + dstPosition,
                        ((INT_ARRAY_INSTANCE) pSource)->values + srcPosition,
                        length * pSourceClassDef->elementByteSize);
                    break;
                case LONG_ARRAY_TYPE:
                    memoryMove(((LONG_ARRAY_INSTANCE) pDest)->values + dstPosition,
                        ((LONG_ARRAY_INSTANCE) pSource)->values + srcPosition,
                        length * pSourceClassDef->elementByteSize);
                    break;
                case SHORT_ARRAY_TYPE:
                    memoryMove(((SHORT_ARRAY_INSTANCE) pDest)->values + dstPosition,
                        ((SHORT_ARRAY_INSTANCE) pSource)->values + srcPosition,
                        length * pSourceClassDef->elementByteSize);
                    break;
                case BYTE_ARRAY_TYPE:
                    memoryMove(((BYTE_ARRAY_INSTANCE) pDest)->values + dstPosition,
                        ((BYTE_ARRAY_INSTANCE) pSource)->values + srcPosition,
                        length * pSourceClassDef->elementByteSize);
                    break;
#if IMPLEMENTS_FLOAT
                case FLOAT_ARRAY_TYPE:
                    memoryMove(((FLOAT_ARRAY_INSTANCE) pDest)->values + dstPosition,
                        ((FLOAT_ARRAY_INSTANCE) pSource)->values + srcPosition,
                        length * pSourceClassDef->elementByteSize);
                    break;
                case DOUBLE_ARRAY_TYPE:
                    memoryMove(((DOUBLE_ARRAY_INSTANCE) pDest)->values + dstPosition,
                        ((DOUBLE_ARRAY_INSTANCE) pSource)->values + srcPosition,
                        length * pSourceClassDef->elementByteSize);
                    break;
#endif
                default:
                    /* should never reach here - if we did, it means that the 
                     * loader and/or the verifier did not do their jobs properly
                     */
                    throwErrorFromNativeAndReturn(ERROR_CODE_VIRTUAL_MACHINE);
            } /* end switch */
        }
    }
    else if(isPrimitiveArrayClass(pDestClassDef)) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ARRAY_STORE);
    }
    else {
        /* object array copy */
        
        OBJECT_ARRAY_INSTANCE pSourceArray = (OBJECT_ARRAY_INSTANCE) pSource;
        OBJECT_ARRAY_INSTANCE pDestArray = (OBJECT_ARRAY_INSTANCE) pDest;

        if(isInstanceOf((COMMON_CLASS_DEF) pDestClassDef, (COMMON_CLASS_DEF) pSourceClassDef)) {
            memoryMove(pDestArray->values + dstPosition, 
                pSourceArray->values + srcPosition, 
                length * pSourceClassDef->elementByteSize);
        }
        else {
            INT32 i;
            OBJECT_INSTANCE element;
            
            /* we need to check the type of each element */
            for(i=0; i<length; i++) {
                element = getObjectFieldObject(pSourceArray->values + (srcPosition + i));
                if(element != NULL && isInstanceOf((COMMON_CLASS_DEF) pDestClassDef, element->header.pClassDef)) {
                    setObjectFieldObject(pDestArray->values + (dstPosition + i), element);
                }
                else {
                    throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ARRAY_STORE);    
                }
            }
        }
    }
    decrementOperandStackNativeBy(5);
    incrementNativeProgramCounterRegular();
}



void javaLangThrowableCreateStackTrace()
{
#if CREATE_THROWABLE_STACK_TRACES
    THROWING_LOCALS
    throwRegisterStruct reg;
    RETURN_CODE ret;

    reg.pc = getPCNative();
    reg.fp = getFPNative();

    ret = createStackTrace(getOperandStackObjectNative(0), &reg);
    if(ret != SUCCESS) {
        throwErrorFromNativeAndReturn(ret);
    }
#endif
    decrementOperandStackNative();
    incrementNativeProgramCounterVirtual();
}


void javaLangThrowablePrintStackTrace()
{
#if CREATE_THROWABLE_STACK_TRACES
    THROWABLE_INSTANCE pObject = getThrowableInstanceFromObjectInstance(getOperandStackObjectNative(0));
    OBJECT_INSTANCE pStackTrace = extractObjectFieldObject(pObject->stackTrace);
    
    enterStackTraceMonitor();
    if(pStackTrace == NULL) {
        psPrintErr("Stack trace unavailable\n");
    }
    else {
        printStackTrace(pStackTrace);
    }
    exitStackTraceMonitor();
#else
    enterStackTraceMonitor();
    psPrintErr("Stack trace unavailable\n");
    exitStackTraceMonitor();
#endif
    decrementOperandStackNative();
    incrementNativeProgramCounterVirtual();
}



void javaLangClassForName()
{
    THROWING_LOCALS
    OBJECT_INSTANCE pObjectString = getOperandStackObjectNative(0);
    STRING_INSTANCE pObjectName;
    char *pName;
    CLASS_INSTANCE pClassInstance;
    classIdentifierStruct classID;
    RETURN_CODE ret;
    UINT16 i, nameLength;

    
    if(pObjectString == NULL) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_NULL_POINTER);
    }
    pObjectName = getStringInstanceFromObjectInstance(pObjectString);
    nameLength = extractPrimitiveFieldInt(pObjectName->length);
    pName = memoryAlloc(nameLength);
    if(pName == NULL) {
        throwErrorFromNativeAndReturn(ERROR_CODE_OUT_OF_MEMORY);
    }
    getUTF8StringFromStringInstance(pObjectName, pName);
    for(i=0; i<nameLength; i++) {
        if(pName[i] == '.') {
            pName[i] = '/';
        }
    }

    ret = getClassParams(pName, (UINT16) extractPrimitiveFieldInt(pObjectName->length), &classID);
    memoryFree(pName);
    if(ret != SUCCESS) {
        throwErrorFromNativeAndReturn(ret);
    }

    ret = obtainClassRef(classID.key, &pClassInstance);
    if(ret != SUCCESS) {
        throwErrorFromNativeAndReturn(ret);
    }
    
    /* as with the invokes, we are incrementing the program counter before a frame is pushed;
     * an initialization frame will be pushed if necessary 
     */
    pushOperandStackObjectNative(getClassObjectFromClassInstance(pClassInstance));
    incrementNativeProgramCounterRegular();
    if(!classIsInitialized(pClassInstance, getCurrentJavaThread())) {
        initializeClassNative(pClassInstance);
    }
}


void javaLangClassNewInstance()
{
    THROWING_LOCALS
    RETURN_CODE ret;
    METHOD_DEF pInitMethod;
    LOADED_CLASS_DEF pClassDef;

    ret = instantiateCollectibleObject(getClassInstanceFromClassObject(getOperandStackObjectNative(0)), getOperandStackObjectPointerNative(0));
    if(ret != SUCCESS) {
        throwErrorFromNativeAndReturn(ret);
    }

    /* now that we have the new object, we need to call <init> */

    /* find the <init> method */
    pClassDef = (LOADED_CLASS_DEF) getOperandStackObjectNative(0)->header.pClassDef;

    
    ret = lookupMethod(pClassDef, initNameType, FALSE, &pInitMethod);
    if(ret != SUCCESS || pInitMethod->base.pOwningClass != pClassDef || isAbstractClass(pClassDef)
        || isArrayClass(pClassDef) || isInterface(pClassDef)) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_INSTANTIATION);
    }
    if(!isClassAccessible(getCurrentClassDefNative(), pClassDef) ||
        !isClassMemberAccessible((COMMON_CLASS_DEF) getCurrentClassDefNative(), (COMMON_CLASS_DEF) (pInitMethod->base.pOwningClass), pInitMethod->classBase.accessFlags)) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_ACCESS);
    }
    incrementNativeProgramCounterVirtual();

    /* The internal init method will catch any thrown exceptions.  We also need it
     * because there may be no operand stack space for the object instance for the init call.
     */
    pushStackFrameNative(pInternalInitMethod);

    /* push another object reference on the stack for the init call */
    pushOperandStackObjectNative(getOperandStackObjectNative(0));

    pushStackFrameNative(pInitMethod);   
}


void javaLangClassGetName()
{
    THROWING_LOCALS
    char *packageName;
    char *name;
    char *full;
    char *arrayDepth;
    UINT16 packageNameLength;
    UINT16 nameLength;
    UINT16 arrayDepthLength;
    UINT16 totalLength;
    UINT16 i;
    INTERNED_STRING_INSTANCE pString;
    RETURN_CODE ret;
    
    COMMON_CLASS_DEF pClassDef = getClassInstanceFromClassObject(getOperandStackObjectNative(0))->pRepresentativeClassDef;
    
    arrayDepthLength = getArrayDepth(getPackageKey(pClassDef));
    arrayDepth = getArrayDepthStringByDepth(arrayDepthLength);
    packageName = getPackageNameStringByKey(getPackageKey(pClassDef), &packageNameLength);
    name = getUTF8StringByKey(getNameKey(pClassDef), &nameLength);
    totalLength = nameLength + packageNameLength + arrayDepthLength + 1;
    full = memoryAlloc(totalLength);
    if(full == NULL) {
        throwErrorFromNativeAndReturn(ERROR_CODE_OUT_OF_MEMORY);
    }
    memoryCopy(full, arrayDepth, arrayDepthLength);
    memoryCopy(full + arrayDepthLength, packageName, packageNameLength);
    full[arrayDepthLength + packageNameLength] = '.';
    memoryCopy(full + arrayDepthLength + packageNameLength + 1, name, nameLength);
    
    for(i=arrayDepthLength; i<packageNameLength; i++) {
        if(full[i] == '/') {
            full[i] = '.';
        }
    }
    ret = internString(full, (UINT16) (arrayDepthLength + nameLength + packageNameLength + 1), &pString);
    memoryFree(full);
    if(ret != SUCCESS) {
        throwErrorFromNativeAndReturn(ret);
    }
    incrementNativeProgramCounterVirtual();
    setOperandStackObjectNative(0, (OBJECT_INSTANCE) &pString->header);
}

void javaLangClassIsAssignableFrom()
{
    THROWING_LOCALS
    OBJECT_INSTANCE pSourceObject;

    pSourceObject = getOperandStackObjectNative(1);
    if(pSourceObject == NULL) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_NULL_POINTER);
    }

    setOperandStackIntNative(1, 
        isInstanceOf(
            getClassInstanceFromClassObject(getOperandStackObjectNative(0))->pRepresentativeClassDef, 
            getClassInstanceFromClassObject(pSourceObject)->pRepresentativeClassDef
            ));
    incrementNativeProgramCounterVirtual();
    decrementOperandStackNative();
}

void javaLangClassIsInstance()
{
    setOperandStackIntNative(1, 
        isObjectInstanceOf(
            getClassInstanceFromClassObject(getOperandStackObjectNative(0))->pRepresentativeClassDef, 
            getOperandStackObjectNative(1)
            ));
    incrementNativeProgramCounterVirtual();
    decrementOperandStackNative();
}

void javaLangClassIsArray()
{
    setOperandStackIntNative(0, isArrayClass(getClassInstanceFromClassObject(getOperandStackObjectNative(0))->pRepresentativeClassDef));
    incrementNativeProgramCounterVirtual();
}

void javaLangClassIsInterface()
{
    setOperandStackIntNative(0, isInterface(getClassInstanceFromClassObject(getOperandStackObjectNative(0))->pRepresentativeClassDef));
    incrementNativeProgramCounterVirtual();
}

void javaLangClassGetResourceAsStream()
{
    THROWING_LOCALS
    OBJECT_INSTANCE pObjectString = getOperandStackObjectNative(1);
    OBJECT_INSTANCE pClassObject = getOperandStackObjectNative(0);
    CLASS_INSTANCE pClassInstance = getClassInstanceFromClassObject(pClassObject);
    STRING_INSTANCE pResName;
    char *pName;
    OBJECT_INSTANCE pInputStream;
    RETURN_CODE ret;

    if(pObjectString == NULL) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_NULL_POINTER);
    }
    pResName = getStringInstanceFromObjectInstance(pObjectString);
    pName = memoryAlloc(extractPrimitiveFieldInt(pResName->length));
    if(pName == NULL) {
        throwErrorFromNativeAndReturn(ERROR_CODE_OUT_OF_MEMORY);
    }
    getUTF8StringFromStringInstance(pResName, pName);
    ret = getResourceAsStream(pClassInstance, pName, (UINT16) extractPrimitiveFieldInt(pResName->length), &pInputStream);
    memoryFree(pName);
    if(ret != SUCCESS) {
        throwErrorFromNativeAndReturn(ret);
    }
    setOperandStackObjectNative(0, pInputStream);
    incrementNativeProgramCounterVirtual();
}

void comMjvmkResourceInputStreamRead()
{
    THROWING_LOCALS
    OBJECT_INSTANCE pInputStreamObject = getOperandStackObjectNative(0);
    RESOURCE_INPUT_STREAM_INSTANCE pResourceInputStreamObject = getResourceInputStreamInstanceFromObjectInstance(pInputStreamObject);
    FILE_ACCESS pFileAccess;
    jumpBufferStruct errorJump;
    
    if(extractPrimitiveFieldInt(pResourceInputStreamObject->isClosed)) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_IO);
    }
    pFileAccess = getResourceFileAccess(pResourceInputStreamObject);
    pFileAccess->pErrorJump = &errorJump;
    if(setJump(errorJump) != 0) {
        pFileAccess->pErrorJump = NULL;
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_IO);
    }
    else {
        if(pFileAccess->pFileAccessMethods->eof(pFileAccess)) {
            setOperandStackIntNative(0, -1);
        }
        else {
            setOperandStackIntNative(0, pFileAccess->pFileAccessMethods->loadByte(pFileAccess));
        }
    }
    incrementNativeProgramCounterRegular();
}

void comMjvmkResourceInputStreamClose()
{
    THROWING_LOCALS
    OBJECT_INSTANCE pInputStreamObject = getOperandStackObjectNative(0);
    RESOURCE_INPUT_STREAM_INSTANCE pResourceInputStreamObject = getResourceInputStreamInstanceFromObjectInstance(pInputStreamObject);
    FILE_ACCESS pFileAccess;
    jumpBufferStruct errorJump;
    BOOLEAN ret;
    
    if(!extractPrimitiveFieldInt(pResourceInputStreamObject->isClosed)) {
        pFileAccess = getResourceFileAccess(pResourceInputStreamObject);
        pFileAccess->pErrorJump = &errorJump;
        if(setJump(errorJump) != 0) {
            pFileAccess->pErrorJump = NULL;
            throwExceptionFromNativeAndReturn(EXCEPTION_CODE_IO);
        }
        else {
            ret = pFileAccess->pFileAccessMethods->close(pFileAccess);
            if(!ret) {
                throwExceptionFromNativeAndReturn(EXCEPTION_CODE_IO);
            }
            insertPrimitiveFieldInt(pResourceInputStreamObject->isClosed, TRUE);
        }
    }
    decrementOperandStackNative();
    incrementNativeProgramCounterRegular();
}

/* Note: if pointers are more than 4 bytes long the following could produce dpulicate hashcodes
 * for different objects, but that's OK by the requirements of the method 
 */
void javaLangObjectHashCode()
{
    setOperandStackIntNative(0, (UINT32) getOperandStackObjectNative(0));
    incrementNativeProgramCounterVirtual();
}

void javaLangObjectWait()
{
    THROWING_LOCALS
    MONITOR pMonitor = getOperandStackObjectNative(0)->header.pMonitor;

    if(pMonitor == NULL || monitorNotOwnedByCurrentJavaThread(pMonitor)) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_MONITOR_STATE);
    }       
    induceJavaThreadSwitch();
    monitorWait(pMonitor, NULL);
    checkForInterruptedExceptionNative();
    incrementNativeProgramCounterVirtual();
    decrementOperandStackNative();
}

void javaLangObjectWaitJI()
{
    THROWING_LOCALS
    MONITOR pMonitor = getOperandStackObjectNative(3)->header.pMonitor;
    jlong timeOutMillis = getOperandStackLongNative(2);
    jint timeOutNanos = getOperandStackIntNative(0);
    timeValueStruct timeOut;
    
    if(jlongLessThan(timeOutMillis, jlongZero) || ((INT32) timeOutNanos) < 0 || ((INT32) timeOutNanos) > 999999) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_ARGUMENT);
    }
    setTimeMillisNanos(&timeOut, timeOutMillis, timeOutNanos);
    if(pMonitor == NULL || monitorNotOwnedByCurrentJavaThread(pMonitor)) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_MONITOR_STATE);
    }
    induceJavaThreadSwitch();
    monitorWait(pMonitor, &timeOut);
    checkForInterruptedExceptionNative();
    incrementNativeProgramCounterVirtual();
    decrementOperandStackNative();
}

void javaLangObjectWaitJ()
{
    THROWING_LOCALS
    MONITOR pMonitor;
    jlong timeOutMillis;
    timeValueStruct timeOut;

    pMonitor = getOperandStackObjectNative(2)->header.pMonitor;
    timeOutMillis = getOperandStackLongNative(1);

    if(jlongLessThan(timeOutMillis, jlongZero)) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_ARGUMENT);
    }
    setTimeMillis(&timeOut, timeOutMillis);
    if(pMonitor == NULL || monitorNotOwnedByCurrentJavaThread(pMonitor)) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_MONITOR_STATE);
    }
    induceJavaThreadSwitch();
    monitorWait(pMonitor, &timeOut);
    checkForInterruptedExceptionNative();
    incrementNativeProgramCounterVirtual();
    decrementOperandStackNative();
}

void javaLangObjectNotify()
{
    THROWING_LOCALS
    MONITOR pMonitor = getOperandStackObjectNative(0)->header.pMonitor;

    if(pMonitor == NULL || monitorNotOwnedByCurrentJavaThread(pMonitor)) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_MONITOR_STATE);
    }       
    monitorNotify(pMonitor);
    checkForJavaThreadSwitch();
    incrementNativeProgramCounterVirtual();
    decrementOperandStackNative();    
}

void javaLangObjectNotifyAll()
{
    THROWING_LOCALS
    MONITOR pMonitor = getOperandStackObjectNative(0)->header.pMonitor;
    
    if(pMonitor == NULL || monitorNotOwnedByCurrentJavaThread(pMonitor)) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_MONITOR_STATE);
    }       
    monitorNotifyAll(pMonitor);
    checkForJavaThreadSwitch();
    incrementNativeProgramCounterVirtual();
    decrementOperandStackNative();    
}

void javaLangObjectGetClass()
{
    setOperandStackObjectNative(0, getClassObjectFromClassInstance(getOperandStackObjectNative(0)->header.pClassInstance));
    incrementNativeProgramCounterVirtual();
}

void javaLangThreadJoin()
{
#if PREEMPTIVE_THREADS
    THROWING_LOCALS
#endif
    THREAD pThread = (THREAD) getThreadStructFromObject(getOperandStackObjectNative(0));
    induceJavaThreadSwitch();
    threadJoin(pThread);
    checkForInterruptedExceptionNative();
    incrementNativeProgramCounterVirtual();
    decrementOperandStackNative();
}

void javaLangThreadSleep()
{
#if PREEMPTIVE_THREADS
    THROWING_LOCALS
#endif
    timeValueStruct elapsedTime;

    jlong millis = getOperandStackLongNative(1);
    setTimeMillis(&elapsedTime, millis);
    induceJavaThreadSwitch();
    threadSleep(&elapsedTime);
    checkForInterruptedExceptionNative();
    incrementNativeProgramCounterRegular();
    decrementOperandStackNativeBy(2);
}

void javaLangThreadIsAlive()
{
    setOperandStackIntNative(0, ((THREAD) getThreadStructFromObject(getOperandStackObjectNative(0)))->state == THREAD_ACTIVE);
    incrementNativeProgramCounterVirtual();
}

void javaLangThreadGetPriority()
{
    setOperandStackIntNative(0, getJavaPriority(getThreadStructFromObject(getOperandStackObjectNative(0))));
    incrementNativeProgramCounterVirtual();
}


void javaLangThreadSetPriority()
{
    THROWING_LOCALS
    INT32 newPriority = getOperandStackIntNative(0);

    if(newPriority < JAVA_PRIORITY_LOWEST || newPriority > JAVA_PRIORITY_HIGHEST) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_ARGUMENT);
    }
    setJavaPriority(getThreadStructFromObject(getOperandStackObjectNative(1)), (UINT8) newPriority);
    checkForJavaThreadSwitch();
    incrementNativeProgramCounterVirtual();
    decrementOperandStackNativeBy(2);
}

void javaLangThreadActiveCount()
{
    pushOperandStackIntNative(getActiveJavaThreadCount());
    incrementNativeProgramCounterRegular();
}

void javaLangThreadCurrentThread()
{
    pushOperandStackObjectNative(getCurrentJavaThread()->pThreadInstance);
    incrementNativeProgramCounterRegular();
}

void javaLangThreadInterrupt()
{
    interruptThread((THREAD) getThreadStructFromObject(getOperandStackObjectNative(0)));
    checkForJavaThreadSwitch();
    incrementNativeProgramCounterVirtual();
    decrementOperandStackNative();
}

void javaLangThreadYield()
{
    strongYield();
    induceJavaThreadSwitch();
    incrementNativeProgramCounterRegular();
}

void javaLangThreadStart()
{
    THROWING_LOCALS
    OBJECT_INSTANCE pThreadObject;
    RETURN_CODE ret;
    
    pThreadObject = getOperandStackObjectNative(0);
    if(containsThreadStruct(getThreadInstanceFromObjectInstance(pThreadObject)) && ((THREAD) getThreadStructFromObject(pThreadObject))->state != THREAD_NOT_STARTED) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ILLEGAL_THREAD_STATE);
    }
    ret = startNewThread(pThreadObject, INITIAL_JAVA_STACK_SIZE);
    if(ret != SUCCESS) {
        throwErrorFromNativeAndReturn(ret);
    }

    /* switch to the new thread right away if it has a higher priority */
    weakYield();
    checkForJavaThreadSwitch();
    decrementOperandStackNative();
    incrementNativeProgramCounterVirtual();
}


/* the charAtInternal methods are private to the java.lang package and therefore no bounds checking
 * or null pointer checking is required.
 */
void javaLangStringCharAtInternal()
{
    STRING_INSTANCE pInstance;

#if QUICKENING_ENABLED
    quickenInstructionStream1(getPCNative(), STRING_CHAR_AT_INTERNAL_QUICK);
#endif
    pInstance = getStringInstanceFromObjectInstance(getOperandStackObjectNative(1));
    setOperandStackIntNative(1, ((CHAR_ARRAY_INSTANCE) extractObjectFieldObject(pInstance->pCharArray))->values[extractPrimitiveFieldInt(pInstance->offset) + getOperandStackIntNative(0)]);
    decrementOperandStackNative();
    incrementNativeProgramCounterRegular();
}


void javaLangStringCharAt()
{
    THROWING_LOCALS
    INT32 stringOffset;
    INT32 stringLength;
    INT32 index;
    OBJECT_INSTANCE pObject;
    STRING_INSTANCE pInstance;

#if QUICKENING_ENABLED
    quickenInstructionStream1(getPCNative(), STRING_CHAR_AT_QUICK);
#endif
    index = getOperandStackIntNative(0);
    pObject = getOperandStackObjectNative(1);
    if(pObject == NULL) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_NULL_POINTER);
    }
    pInstance = getStringInstanceFromObjectInstance(pObject);
    stringOffset = extractPrimitiveFieldInt(pInstance->offset);
    stringLength = extractPrimitiveFieldInt(pInstance->length);
    if(index < 0 || index >= stringLength) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_STRING_INDEX_OUT_OF_BOUNDS);
    }
    setOperandStackIntNative(1, ((CHAR_ARRAY_INSTANCE) extractObjectFieldObject(pInstance->pCharArray))->values[stringOffset + index]);
    decrementOperandStackNative();
    incrementNativeProgramCounterRegular();
}


void javaLangIntegerValue()
{
    THROWING_LOCALS
    OBJECT_INSTANCE pObject;

#if QUICKENING_ENABLED
    /* we cannot use the quickened instruction unless this is a method call of
     * instruction length 3, ie it is not an interface method call.  The only way that any native
     * method that resolves to this function could ever be an interface call is if:
     * - it is in a non-final class
     * - it is public and not static
     * - someone creates an interface which includes a method with the same name-type
     * - someonce subclasses the class containing the native method, so that the native method ends up
     *   being the implmentation of a method in the interface, which was implemented by the sub-class
     *
     * So you can see it is pretty rare occurence that this method call is not quickened.
     */
    switch(*getPCNative()) {
        case INVOKEINTERFACE:
        case INVOKEINTERFACE_QUICK:
            break;
        default:
            quickenInstructionStream1(getPCNative(), GETFIELD_0_QUICK_PRIMITIVE);
    }
    
#endif
    pObject = getOperandStackObjectNative(0);
    if(pObject == NULL) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_NULL_POINTER);
    }
    setOperandStackIntNative(0, getPrimitiveFieldInt(getPrimitiveField(pObject, 0)));


    incrementNativeProgramCounterVirtual();
}


void javaLangMathIMax() 
{
    INT32 a = getOperandStackIntNative(1);
    INT32 b = getOperandStackIntNative(0);
#if QUICKENING_ENABLED
    quickenInstructionStream2(getPCNative(), MATH_QUICK, IMAX);
#endif
    if(a < b) {
        setOperandStackIntNative(1, b);
    }
    decrementOperandStackNative();
    incrementNativeProgramCounterRegular();
}

void javaLangMathIMin() 
{
    INT32 a = getOperandStackIntNative(1);
    INT32 b = getOperandStackIntNative(0);
#if QUICKENING_ENABLED
    quickenInstructionStream2(getPCNative(), MATH_QUICK, IMIN);
#endif
    if(a > b) {
        setOperandStackIntNative(1, b);
    }
    decrementOperandStackNative();
    incrementNativeProgramCounterRegular();
}

void javaLangMathIAbs() 
{
    INT32 a = getOperandStackIntNative(0);
#if QUICKENING_ENABLED
    quickenInstructionStream2(getPCNative(), MATH_QUICK, IABS);
#endif
    if(a < 0) {
        setOperandStackIntNative(0, -a);
    }
    incrementNativeProgramCounterRegular();
}

void javaLangMathLMax() 
{
    jlong a = getOperandStackLongNative(3);
    jlong b = getOperandStackLongNative(1);
#if QUICKENING_ENABLED
    quickenInstructionStream2(getPCNative(), MATH_QUICK, LMAX);
#endif
    if(jlongLessThan(a, b)) {
        setOperandStackLongNative(3, b);
    }
    decrementOperandStackNativeBy(2);
    incrementNativeProgramCounterRegular();
}

void javaLangMathLMin() 
{
    jlong a = getOperandStackLongNative(3);
    jlong b = getOperandStackLongNative(1);
#if QUICKENING_ENABLED
    quickenInstructionStream2(getPCNative(), MATH_QUICK, LMIN);
#endif
    if(jlongGreaterThan(a, b)) {
        setOperandStackLongNative(3, b);
    }
    decrementOperandStackNativeBy(2);
    incrementNativeProgramCounterRegular();
}

void javaLangMathLAbs() 
{
    jlong a = getOperandStackLongNative(1);
#if QUICKENING_ENABLED
    quickenInstructionStream2(getPCNative(), MATH_QUICK, LABS);
#endif
    if(jlongLessThan(a, jlongZero)) {
        jlongNegate(a);
        setOperandStackLongNative(1, a);
    }
    incrementNativeProgramCounterRegular();
}




/* the elementAtInternal method is private to the java.util package and therefore no bounds checking
 * or null pointer checking is required.
 */
void javaUtilVectorElementAtInternal()
{
    VECTOR_INSTANCE pInstance;

#if QUICKENING_ENABLED
    quickenInstructionStream1(getPCNative(), VECTOR_ELEMENT_AT_INTERNAL_QUICK);
#endif
    pInstance = getVectorInstanceFromObjectInstance(getOperandStackObjectNative(1));
    setOperandStackObjectNative(1, extractObjectFieldObject(((OBJECT_ARRAY_INSTANCE) extractObjectFieldObject(pInstance->objectArray))->values[getOperandStackIntNative(0)]));
    decrementOperandStackNative();
    incrementNativeProgramCounterRegular();
}

/* note that any locking for output stream or error stream writing should be done at the java level */

void comMjvmkStandardOutputStreamWriteBytes()
{
    BYTE_ARRAY_INSTANCE pByteArray = (BYTE_ARRAY_INSTANCE) getOperandStackObjectNative(2);
    INT32 offset = getOperandStackIntNative(1);
    INT32 length = getOperandStackIntNative(0);
    INT32 arrayLength = pByteArray->header.arrayLength;
    INT32 i;
    THROWING_LOCALS

    /* check the indices */
    if (length < 0 || offset < 0
          || offset + length > arrayLength) {
        throwExceptionFromNativeAndReturn(EXCEPTION_CODE_ARRAY_INDEX_OUT_OF_BOUNDS);
    }

    for(i=offset; i<length; i++) {
        psPutChar(pByteArray->values[i]);
    }
    decrementOperandStackNativeBy(3);
    incrementNativeProgramCounterVirtual();
}


void comMjvmkStandardOutputStreamWriteByteArray()
{
    BYTE_ARRAY_INSTANCE pByteArray = (BYTE_ARRAY_INSTANCE) getOperandStackObjectNative(0);
    UINT16 length;
    UINT16 i;

    length = pByteArray->header.arrayLength;
    for(i=0; i<length; i++) {
        psPutChar(pByteArray->values[i]);
    }
    decrementOperandStackNativeBy(2);
    incrementNativeProgramCounterVirtual();
}

void comMjvmkStandardOutputStreamWrite()
{
    psPutChar(getOperandStackIntNative(0));
    decrementOperandStackNativeBy(2);
    incrementNativeProgramCounterVirtual();
}

void comMjvmkStandardOutputStreamFlush()
{
    psFlushOut();
    decrementOperandStackNative();
    incrementNativeProgramCounterVirtual();
}

void comMjvmkStandardErrorStreamWrite()
{
    psPutCharErr(getOperandStackIntNative(0));
    decrementOperandStackNativeBy(2);
    incrementNativeProgramCounterVirtual();
}
