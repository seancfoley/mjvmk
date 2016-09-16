
#include "thread/schedule.h"
#include "memory/memoryAlloc.h"
#include "elements/javaParameters.h"
#include "nativeTable.h"

/* the lookup struct allows one to find the index in the table of entry structs */
typedef struct nativeFunctionLookupStruct {
    NamePackageKey classKey;
    NameTypeKey methodKey;
    INT16 nativeFunctionIndex;    
} nativeFunctionLookupStruct, *NATIVE_FUNCTION_LOOKUP;



/* 
 * There are two tables, one that can be romized, the lookup table,
 * and one that cannot, the entry table.  The reason for this is that
 * the value of a function pointer can change with every build, and
 * is thus not something you would wish to romize.
 *
 * However, by mapping native methods to an index, the index can be
 * romized since it will not change with each native function, 
 * while the function ptr that the index maps to may change with 
 * each build.
 *
 * Both tables can grow dynamically, allowing you to dynamically 
 * map native methods to functions.
 */



static NATIVE_FUNCTION_LOOKUP nativeFunctionLookupTable = NULL;
static UINT16 nativeFunctionLookupTableSize = 0;

NATIVE_FUNCTION_ENTRY nativeFunctionEntryTable = NULL;
static UINT16 nativeFunctionEntryTableSize = 0;

#define NATIVE_TABLE_BLOCK_SIZE ((UINT16) 64)



static RETURN_CODE addNativeFunctionLookup(NamePackageKey classKey, NameTypeKey methodKey,
                                           UINT16 nativeFunctionIndex) 
{
    /* re-size the table if necessary */
    if(nativeFunctionIndex >= nativeFunctionLookupTableSize) {
        RETURN_CODE ret;
        
        ret = resizeTable(&nativeFunctionLookupTable, &nativeFunctionLookupTableSize, 
                    sizeof(nativeFunctionLookupStruct), NATIVE_TABLE_BLOCK_SIZE,
                    (UINT16) (nativeFunctionIndex + 1));
        if(ret != SUCCESS) {
            return ret;
        }
    }

    /* add the entry to the table */
    nativeFunctionLookupTable[nativeFunctionIndex].classKey = classKey;
    nativeFunctionLookupTable[nativeFunctionIndex].methodKey = methodKey;
    nativeFunctionLookupTable[nativeFunctionIndex].nativeFunctionIndex = nativeFunctionIndex;
    return SUCCESS;
}

static RETURN_CODE addNativeFunctionEntry(INT16 nativeFunctionIndex, NATIVE_FUNCTION pNativeFunction) 
{
    /* re-size the table if necessary */
    if(nativeFunctionIndex >= nativeFunctionEntryTableSize) {
        RETURN_CODE ret;
        
        ret = resizeTable(&nativeFunctionEntryTable, &nativeFunctionEntryTableSize, 
                    sizeof(nativeFunctionEntryStruct), NATIVE_TABLE_BLOCK_SIZE,
                    (INT8) (nativeFunctionIndex + 1));
        if(ret != SUCCESS) {
            return ret;
        }
    }
    nativeFunctionEntryTable[nativeFunctionIndex].pNativeFunction = pNativeFunction;
    return SUCCESS;
}

static UINT16 nativeFunctionCount = 0;


#if PREEMPTIVE_THREADS

monitorStruct nativeTableMonitor = {0, NULL, NULL, NULL};
MONITOR pNativeTableMonitor = &nativeTableMonitor;
#define enterNativeTableMonitor() monitorEnter(pNativeTableMonitor)
#define exitNativeTableMonitor() monitorExit(pNativeTableMonitor)

#else

#define enterNativeTableMonitor()
#define exitNativeTableMonitor()

#endif



RETURN_CODE addNativeFunction(NamePackageKey classKey, NameTypeKey methodKey, NATIVE_FUNCTION pNativeFunction, UINT16 *index) 
{
    RETURN_CODE ret;

    enterNativeTableMonitor();
    ret = addNativeFunctionLookup(classKey, methodKey, nativeFunctionCount);
    if(ret != SUCCESS) {
        return ret;
    }
    ret = addNativeFunctionEntry(nativeFunctionCount, pNativeFunction);
    if(ret != SUCCESS) {
        return ret;
    }
    *index = nativeFunctionCount;
    nativeFunctionCount++;
    exitNativeTableMonitor();
    
    return SUCCESS;
}

/* returns -1 if not found */
RETURN_CODE getNativeFunctionIndex(NamePackageKey classKey, NameTypeKey methodKey, UINT16 *index) 
{
    NATIVE_FUNCTION_LOOKUP lookup;
    INT16 i;

    for(i=0; i<nativeFunctionCount; i++) {
        lookup = nativeFunctionLookupTable + i;
        if(lookup->classKey == classKey && lookup->methodKey == methodKey) {
            *index = i;
            return SUCCESS;
        }
    }
    return ERROR_CODE_UNSATISFIED_LINK;
}






