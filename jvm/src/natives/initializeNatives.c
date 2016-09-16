

#include "string/stringKeys.h"
#include "class/nativeTable.h"
#include "natives.h"
#include "initializeNatives.h"




/*
 * This page provides the initialization necessary to support the base load of 
 * native methods in this implementation of CLDC.  Programmers may add their
 * own natives to the build load by simply adding to the table below.  Or they
 * may add natives dynamically by making calls to addNativeFunction.
 *
 */

typedef struct baseNativeFunction {
    NamePackageKey classKey;
    NameKey methodNameKey;
    NameKey methodTypeKey;
    NATIVE_FUNCTION pNativeFunction;   /* C function to implement Java method */
} baseNativeFunction, *BASE_NATIVE_FUNCTION;



RETURN_CODE initializeNativeTable() 
{
    UINT16 counter = 0, functionIndex;
    BASE_NATIVE_FUNCTION pFunction;
    RETURN_CODE ret;
    NameType method;
    
    
    baseNativeFunction baseNativeFunctionTable[] = {
        
        {javaLangObjectKey.namePackageKey, getClassString.nameKey, javaLangClassType.nameKey, javaLangObjectGetClass},
        {javaLangObjectKey.namePackageKey, hashCodeString.nameKey, iType.nameKey, javaLangObjectHashCode},
        {javaLangObjectKey.namePackageKey, notifyString.nameKey, vType.nameKey, javaLangObjectNotify},
        {javaLangObjectKey.namePackageKey, notifyAllString.nameKey, vType.nameKey, javaLangObjectNotifyAll},
        {javaLangObjectKey.namePackageKey, waitString.nameKey, vType.nameKey, javaLangObjectWait},
        {javaLangObjectKey.namePackageKey, waitString.nameKey, jvType.nameKey, javaLangObjectWaitJ},
        {javaLangObjectKey.namePackageKey, waitString.nameKey, jivType.nameKey, javaLangObjectWaitJI},
        
        {javaLangClassKey.namePackageKey, isInterfaceString.nameKey, zType.nameKey, javaLangClassIsInterface},
        {javaLangClassKey.namePackageKey, isArrayString.nameKey, zType.nameKey, javaLangClassIsArray},
        {javaLangClassKey.namePackageKey, isInstanceString.nameKey, javaLangObjectZType.nameKey, javaLangClassIsInstance},
        {javaLangClassKey.namePackageKey, isAssignableFromString.nameKey, javaLangClassZType.nameKey, javaLangClassIsAssignableFrom},
        {javaLangClassKey.namePackageKey, getNameString.nameKey, javaLangStringType.nameKey, javaLangClassGetName},
        {javaLangClassKey.namePackageKey, newInstanceString.nameKey, javaLangObjectType.nameKey, javaLangClassNewInstance},
        {javaLangClassKey.namePackageKey, forNameString.nameKey, javaLangStringJavaLangClassType.nameKey, javaLangClassForName},
        {javaLangClassKey.namePackageKey, getResourceAsStreamString.nameKey, javaLangStringjavaIoInputStreamType.nameKey, javaLangClassGetResourceAsStream},
        
        {javaLangThrowableKey.namePackageKey, printStackTraceString.nameKey, vType.nameKey, javaLangThrowablePrintStackTrace},
        {javaLangThrowableKey.namePackageKey, createStackTraceString.nameKey, vType.nameKey, javaLangThrowableCreateStackTrace},
        
        {javaLangSystemKey.namePackageKey, identityHashCodeString.nameKey, javaLangObjectIType.nameKey, javaLangObjectHashCode},
        {javaLangSystemKey.namePackageKey, currentTimeMillisString.nameKey, jType.nameKey, javaLangSystemCurrentTimeMillis},
        {javaLangSystemKey.namePackageKey, arraycopyString.nameKey, javaLangObjectIjavaLangObjectIIVType.nameKey, javaLangSystemArrayCopy},
        {javaLangSystemKey.namePackageKey, getPropertyString.nameKey, javaLangStringjavaLangStringType.nameKey, javaLangSystemGetProperty},
        

        {javaLangRuntimeKey.namePackageKey, freeMemoryString.nameKey, jType.nameKey, javaLangRuntimeFreeMemory},
        {javaLangRuntimeKey.namePackageKey, totalMemoryString.nameKey, jType.nameKey, javaLangRuntimeTotalMemory},
        {javaLangRuntimeKey.namePackageKey, gcString.nameKey, vType.nameKey, javaLangRuntimeGC},
        {javaLangRuntimeKey.namePackageKey, exitString.nameKey, ivType.nameKey, javaLangRuntimeExit},
        
        /* the length is the first field in the String class */
        {javaLangStringKey.namePackageKey, lengthString.nameKey, iType.nameKey, javaLangIntegerValue},
        {javaLangStringKey.namePackageKey, charAtString.nameKey, icType.nameKey, javaLangStringCharAt},
        {javaLangStringKey.namePackageKey, charAtInternalString.nameKey, icType.nameKey, javaLangStringCharAtInternal},
        
        /* the length is the first field in the StringBuffer class */
        {javaLangStringBufferKey.namePackageKey, lengthString.nameKey, iType.nameKey, javaLangIntegerValue},

        /* the value of the first field in all the primitive wrapper classes */
        {javaLangBooleanKey.namePackageKey, booleanValueString.nameKey, zType.nameKey, javaLangIntegerValue},
        {javaLangByteKey.namePackageKey, byteValueString.nameKey, bType.nameKey, javaLangIntegerValue},
        {javaLangShortKey.namePackageKey, shortValueString.nameKey, sType.nameKey, javaLangIntegerValue},
        {javaLangCharacterKey.namePackageKey, charValueString.nameKey, cType.nameKey, javaLangIntegerValue},
        {javaLangIntegerKey.namePackageKey, intValueString.nameKey, iType.nameKey, javaLangIntegerValue},
        
        {javaLangMathKey.namePackageKey, minString.nameKey, iiiType.nameKey, javaLangMathIMax},
        {javaLangMathKey.namePackageKey, maxString.nameKey, iiiType.nameKey, javaLangMathIMin},
        {javaLangMathKey.namePackageKey, absString.nameKey, iiType.nameKey, javaLangMathIAbs},

        {javaLangMathKey.namePackageKey, minString.nameKey, jjjType.nameKey, javaLangMathLMax},
        {javaLangMathKey.namePackageKey, maxString.nameKey, jjjType.nameKey, javaLangMathLMin},
        {javaLangMathKey.namePackageKey, absString.nameKey, jjType.nameKey, javaLangMathLAbs},
        
        {javaLangThreadKey.namePackageKey, startString.nameKey, vType.nameKey, javaLangThreadStart},
        {javaLangThreadKey.namePackageKey, yieldString.nameKey, vType.nameKey, javaLangThreadYield},
        {javaLangThreadKey.namePackageKey, currentThreadString.nameKey, javaLangThreadType.nameKey, javaLangThreadCurrentThread},
        {javaLangThreadKey.namePackageKey, sleepString.nameKey, jvType.nameKey, javaLangThreadSleep},
        {javaLangThreadKey.namePackageKey, isAliveString.nameKey, zType.nameKey, javaLangThreadIsAlive},
        {javaLangThreadKey.namePackageKey, setPriorityString.nameKey, ivType.nameKey, javaLangThreadSetPriority},
        {javaLangThreadKey.namePackageKey, getPriorityString.nameKey, iType.nameKey, javaLangThreadGetPriority},
        {javaLangThreadKey.namePackageKey, activeCountString.nameKey, iType.nameKey, javaLangThreadActiveCount},
        {javaLangThreadKey.namePackageKey, joinString.nameKey, vType.nameKey, javaLangThreadJoin},
        {javaLangThreadKey.namePackageKey, interruptString.nameKey, vType.nameKey, javaLangThreadInterrupt},

        {javaUtilHashtableKey.namePackageKey, sizeString.nameKey, iType.nameKey, javaLangIntegerValue},
        {javaUtilVectorKey.namePackageKey, sizeString.nameKey, iType.nameKey, javaLangIntegerValue},
        {javaUtilVectorKey.namePackageKey, internalElementAtString.nameKey, iJavaLangObjectType.nameKey, javaUtilVectorElementAtInternal},
        
        {comMjvmkStandardOutputStreamKey.namePackageKey, writeString.nameKey, ivType.nameKey, comMjvmkStandardOutputStreamWrite},
        {comMjvmkStandardOutputStreamKey.namePackageKey, flushString.nameKey, vType.nameKey, comMjvmkStandardOutputStreamFlush},
        {comMjvmkStandardOutputStreamKey.namePackageKey, writeString.nameKey, abiivType.nameKey, comMjvmkStandardOutputStreamWriteBytes},
        {comMjvmkStandardOutputStreamKey.namePackageKey, writeString.nameKey, abvType.nameKey, comMjvmkStandardOutputStreamWriteByteArray},
        
        {comMjvmkStandardErrorStreamKey.namePackageKey, writeString.nameKey, ivType.nameKey, comMjvmkStandardErrorStreamWrite},
        
        {comMjvmkResourceInputStreamKey.namePackageKey, readString.nameKey, iType.nameKey, comMjvmkResourceInputStreamRead},
        {comMjvmkResourceInputStreamKey.namePackageKey, closeString.nameKey, vType.nameKey, comMjvmkResourceInputStreamClose},
        
        {0, 0, 0, NULL}
    };

    
    while(TRUE) {

        pFunction = baseNativeFunctionTable + counter;
        if(pFunction->pNativeFunction == NULL) { /* reached the end of the table */
            return SUCCESS;
        }
        method.nt.nameKey = pFunction->methodNameKey;
        method.nt.typeKey = pFunction->methodTypeKey;
        ret = addNativeFunction(pFunction->classKey, method.nameTypeKey, pFunction->pNativeFunction, &functionIndex);
        if(ret != SUCCESS) {
            return ret;
        }
        counter++;
    }
}

