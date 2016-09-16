
#include "interpret/instructions.h"
#include "thread/schedule.h"
#include "memory/memoryAlloc.h"
#include "string/stringKeys.h"
#include "instructionDescs.h"
#include "methodInvokeFilter.h"




typedef struct filteredMethodStruct {
    NamePackageKey classKey;
    NameKey methodNameKey;
    NameKey methodTypeKey;
    UINT8 newByteCodeLength;          /* must be at least as small as the length of the method invoke, which is either 3 or 5 bytes */
    byte newBytecodes[5]; /* since we are replacing invokes, the bytecode sequence will be either 3 or 5 bytes */
} filteredMethodStruct, *FILTERED_METHOD;


typedef struct filteredMethodLookupStruct {
    NamePackageKey classKey;
    NameTypeKey methodKey;
    UINT8 newByteCodeLength;
    PBYTE newBytecodes; /* Will point to an array of length 3 */
} filteredMethodLookupStruct, *FILTERED_METHOD_LOOKUP;


#define FILTERED_TABLE_BLOCK_SIZE 6

static FILTERED_METHOD_LOOKUP filteredMethodLookupTable = NULL;
static UINT16 filteredMethodCount = 0;
static UINT16 filteredMethodLookupTableSize = 0;
NamePackageKey classesOrFilter = 0;
NamePackageKey classesAndFilter = 0xffffffff;


#if PREEMPTIVE_THREADS

monitorStruct invokeTableMonitor = {0, NULL, NULL, NULL};
MONITOR pInvokeTableMonitor = &invokeTableMonitor;
#define enterInvokeTableMonitor() monitorEnter(pInvokeTableMonitor)
#define exitInvokeTableMonitor() monitorExit(pInvokeTableMonitor)

#else

#define enterInvokeTableMonitor()
#define exitInvokeTableMonitor()

#endif


RETURN_CODE addFilteredMethod(NamePackageKey classKey, NameTypeKey methodKey, UINT8 length, PBYTE newBytecodes) 
{
    enterInvokeTableMonitor();

    /* re-size the table if necessary */
    if(filteredMethodCount >= filteredMethodLookupTableSize) {
        RETURN_CODE ret = resizeTable(&filteredMethodLookupTable, &filteredMethodLookupTableSize, 
                    sizeof(filteredMethodLookupStruct), FILTERED_TABLE_BLOCK_SIZE,
                    (UINT16) (filteredMethodCount + 1));
        if(ret != SUCCESS) {
            return ret;
        }
    }

    /* add the entry to the table */
    filteredMethodLookupTable[filteredMethodCount].classKey = classKey;
    filteredMethodLookupTable[filteredMethodCount].methodKey = methodKey; /* ANSI union assignment */
    filteredMethodLookupTable[filteredMethodCount].newBytecodes = newBytecodes;
    filteredMethodLookupTable[filteredMethodCount].newByteCodeLength = length;
    filteredMethodCount++;

    exitInvokeTableMonitor();
   
    return SUCCESS;
}

//TODO: filter the invokes into their quickened counterparts

RETURN_CODE initializeFilteredMethods() 
{
    UINT16 counter = 0;
    FILTERED_METHOD pFilteredMethod;
    RETURN_CODE ret;
    NameType method;


    /* invokes on interfaces can be replaced by up to 5 bytes, other invokes by a maximum of 3 bytes */
        
    filteredMethodStruct filteredMethodTable[] = {
        {javaLangStringKey.namePackageKey, lengthString.nameKey, iType.nameKey, 1, {GETFIELD_0_QUICK_PRIMITIVE}},
        {javaLangStringBufferKey.namePackageKey, lengthString.nameKey, iType.nameKey, 1, {GETFIELD_0_QUICK_PRIMITIVE}},
        {javaLangBooleanKey.namePackageKey, booleanValueString.nameKey, zType.nameKey, 1, {GETFIELD_0_QUICK_PRIMITIVE}},
        {javaLangByteKey.namePackageKey, byteValueString.nameKey, bType.nameKey, 1, {GETFIELD_0_QUICK_PRIMITIVE}},
        {javaLangShortKey.namePackageKey, shortValueString.nameKey, sType.nameKey, 1, {GETFIELD_0_QUICK_PRIMITIVE}},
        {javaLangCharacterKey.namePackageKey, charValueString.nameKey, cType.nameKey, 1, {GETFIELD_0_QUICK_PRIMITIVE}},
        {javaLangIntegerKey.namePackageKey, intValueString.nameKey, iType.nameKey, 1, {GETFIELD_0_QUICK_PRIMITIVE}},
        {javaUtilHashtableKey.namePackageKey, sizeString.nameKey, iType.nameKey, 1, {GETFIELD_0_QUICK_PRIMITIVE}},
        {javaUtilVectorKey.namePackageKey, sizeString.nameKey, iType.nameKey, 1, {GETFIELD_0_QUICK_PRIMITIVE}},
        {javaLangStringKey.namePackageKey, charAtString.nameKey, icType.nameKey, 1, {STRING_CHAR_AT_QUICK}},
        {javaLangStringKey.namePackageKey, charAtInternalString.nameKey, icType.nameKey, 1, {STRING_CHAR_AT_INTERNAL_QUICK}},
        {javaLangMathKey.namePackageKey, minString.nameKey, iiiType.nameKey, 2, {MATH_QUICK, IMIN}},
        {javaLangMathKey.namePackageKey, maxString.nameKey, iiiType.nameKey, 2, {MATH_QUICK, IMAX}},
        {javaLangMathKey.namePackageKey, absString.nameKey, iiType.nameKey, 2, {MATH_QUICK, IABS}},
        {javaLangMathKey.namePackageKey, minString.nameKey, jjjType.nameKey, 2, {MATH_QUICK, LMIN}},
        {javaLangMathKey.namePackageKey, maxString.nameKey, jjjType.nameKey, 2, {MATH_QUICK, LMAX}},
        {javaLangMathKey.namePackageKey, absString.nameKey, jjType.nameKey, 2, {MATH_QUICK, LABS}},
        {javaUtilVectorKey.namePackageKey, internalElementAtString.nameKey, iJavaLangObjectType.nameKey, 1, {VECTOR_ELEMENT_AT_INTERNAL_QUICK}},
        {0, 0, 0, 0, {0, 0, 0, 0, 0}}
    };




    do {
        pFilteredMethod = filteredMethodTable + counter;
        if(pFilteredMethod->newByteCodeLength == 0) {
            /* reached the end of the table */
            return SUCCESS;
        }
        method.nt.nameKey = pFilteredMethod->methodNameKey;
        method.nt.typeKey = pFilteredMethod->methodTypeKey;
        
        ret = addFilteredMethod(pFilteredMethod->classKey, method.nameTypeKey, 
            pFilteredMethod->newByteCodeLength, pFilteredMethod->newBytecodes);
        
        if(ret != SUCCESS) {
            return ret;
        }

        /* 
         * This trick allows us to quickly disqualify most methods for filtering.
         * It is more effective if filteredMethodClasses is a shorter array.
         */
        classesOrFilter |= pFilteredMethod->classKey;
        classesAndFilter &= pFilteredMethod->classKey;
        
        counter++;
    } while(TRUE);
    return SUCCESS;
}

BOOLEAN substituteMethodInvoke(NamePackageKey classKey, NameTypeKey methodKey, INSTRUCTION instruction) 
{
    UINT8 i;
    FILTERED_METHOD_LOOKUP pFilteredMethodLookup;

    for(i=0; i<filteredMethodCount; i++) {
        pFilteredMethodLookup = filteredMethodLookupTable + i;
    
        if(methodKey == pFilteredMethodLookup->methodKey && classKey == pFilteredMethodLookup->classKey) {
            /* substitute the byte codes for the method call */
            UINT8 j;
            UINT8 length = pFilteredMethodLookup->newByteCodeLength;
            UINT8 invokeLength = getInstructionLength(*instruction);
            
            ASSERT(length <= invokeLength);

            for(j=0; j<invokeLength; j++) {
                if(j<length) {
                    instruction[j] = pFilteredMethodLookup->newBytecodes[j];
                }
                else {
                    instruction[j] = NOP;
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}


