#ifndef LOADER_H
#define LOADER_H

#include "elements/base.h"
#include "interpret/returnCodes.h"
#include "class/typeCheck.h"
#include "resolution/file/fileAccess.h"
#include "iterativeLoad.h"

/* The following four structs are used to collect all the info from the class file */


typedef struct fieldDataStruct {
    UINT16 accessFlags;
    NameType nameType;
    BOOLEAN isConstant;
    UINT16 constantConstantPoolIndex;
    BOOLEAN isObject;
    BOOLEAN isDoublePrimitive;
    BOOLEAN isStatic;
} fieldDataStruct, *FIELD_DATA;


typedef struct methodDataStruct {
    UINT16 accessFlags;
    NameType nameType;
    UINT16 exceptionHandlerCount;
    UINT16 maxStack;
    UINT16 maxLocals;
    UINT32 codeLength;
    BOOLEAN isVirtual;
    BOOLEAN isAbstract;
    BOOLEAN isStatic;
    BOOLEAN isNative;
    BOOLEAN isSynchronized;
#if EARLY_NATIVE_LINKING
    UINT16 nativeIndex;
#endif
    UINT16 parameterVarCount;
    INSTRUCTION pCode;
    UINT32 *pMethodParameterMap;
    THROWABLE_HANDLER pExceptionHandlers;
    METHOD_DEF pOverriddenMethod;
    UINT16 superClassOverriddenIndex; /* index of virtual method overridden in super class */
} methodDataStruct, *METHOD_DATA;


typedef struct loadDataStruct {
    MONITOR pMonitor; /* used for locking while we load */
    NamePackage key;
    UINT16 accessFlags;
    BOOLEAN isInterface;
    BOOLEAN isAbstract;
    LOADED_CLASS_DEF pSuperClass;
    UINT16 superVirtualCount; /* number of overridable methods in super class */
    UINT16 superInterfaceCount;
    LOADED_INTERFACE_DEF *pSuperInterfaces;
    UINT16 constantFieldCount;
    UINT16 doubleStaticFieldCount;
    UINT16 objectStaticFieldCount; /* static fields which are objects (including arrays) */
    UINT16 staticFieldCount;
    UINT16 instanceFieldCount;
    UINT16 newFieldCount;
    FIELD_DATA pFields;
    UINT16 abstractMethodCount;
    UINT16 nativeMethodCount;
    UINT16 staticMethodCount;
    UINT16 instanceMethodCount;
    UINT16 virtualMethodCount; /* can override super class methods, can be overridden */
    UINT16 finalVirtualMethodCount; /* can override super class methods, but cannot be overridden */
    UINT16 newMethodCount;
    UINT16 totalExceptionHandlers;
    UINT16 totalExceptionHandlerLists;
    UINT32 totalCodeLength;
    UINT32 totalParameterMapsSize;
    METHOD_DATA clinitMethod;
    METHOD_DATA pMethods;
    UINT16 overridenCount;     /* methods that have overridden a super class method */
    
} loadDataStruct, *LOAD_DATA;


#define isSuperClassOrInterfaceAccessible(currentPackageKey, pTargetClassDef)           \
    (isPublicClass(pTargetClassDef) ||                                                  \
    currentPackageKey == getPackageKey(pTargetClassDef))


RETURN_CODE completeClassLoad(PRELOAD_DATA);


#endif

