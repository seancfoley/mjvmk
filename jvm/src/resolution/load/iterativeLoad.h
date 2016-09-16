#ifndef ITERATIVELOAD_H
#define ITERATIVELOAD_H

#include "string/stringPool.h"
#include "resolution/file/fileAccess.h"

typedef union firstPassConstantPoolEntryUnion firstPassConstantPoolEntryUnion, *FIRST_PASS_CONSTANT_POOL_ENTRY;

typedef FIRST_PASS_CONSTANT_POOL_ENTRY FIRST_PASS_CONSTANT_POOL;


typedef struct preloadDataStruct preloadDataStruct, *PRELOAD_DATA;

typedef struct preloadDataStruct {
    classIdentifierStruct classID;
    classIdentifierStruct superClassID;
    PRELOAD_DATA pChildLoad;
    CLASS_ID pSuperInterfaces;
    UINT16 superInterfaceCount;

    FIRST_PASS_CONSTANT_POOL pFirstConstantPool;
    CONSTANT_POOL pConstantPool;
    BOOLEAN isInterface;
    CLASS_ENTRY pClassEntry;
    FILE_ACCESS pFileAccess;
    UINT16 accessFlags;
    CLASS_INSTANCE pClassInstance;
    LOADED_CLASS_DEF pClassDef;
#if PREEMPTIVE_THREADS
    MONITOR pMonitor;
#endif
};

RETURN_CODE iterativeLoadClass(CLASS_ID pClassID, CLASS_ENTRY *ppClassEntry);
RETURN_CODE loadClass(NamePackage classKey, CLASS_ENTRY *ppClassEntry);
RETURN_CODE initializeLoader();


#endif
