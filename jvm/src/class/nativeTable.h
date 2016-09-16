
#ifndef NATIVETABLE_H
#define NATIVETABLE_H

#include "elements/base.h"
#include "interpret/returnCodes.h"


/* All natives take no arguments and return nothing */
typedef void (*NATIVE_FUNCTION)();

RETURN_CODE addNativeFunction(NamePackageKey classKey, NameTypeKey methodKey, 
                              NATIVE_FUNCTION pNativeFunction, UINT16 *index);

/* this index is required for the loader and is stored in the method structure */
RETURN_CODE getNativeFunctionIndex(NamePackageKey classKey, NameTypeKey methodKey, UINT16 *index);

typedef struct nativeFunctionEntryStruct
{
    NATIVE_FUNCTION pNativeFunction;
} nativeFunctionEntryStruct, *NATIVE_FUNCTION_ENTRY;

extern NATIVE_FUNCTION_ENTRY nativeFunctionEntryTable;

/* When the native is called, the index is used to locate the function */
#define getNativeFunction(index) nativeFunctionEntryTable[index].pNativeFunction

#endif