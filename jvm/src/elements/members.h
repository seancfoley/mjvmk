#ifndef MEMBERS_H
#define MEMBERS_H

#include "keys.h"



typedef struct loadedClassDefStruct loadedClassDefStruct, *LOADED_CLASS_DEF;
typedef struct loadedInterfaceDefStruct loadedInterfaceDefStruct, *LOADED_INTERFACE_DEF;


typedef struct fieldDefStruct {
    UINT16 fieldIndex; /* field data table index (longs and doubles take up two indices) */
    UINT16 accessFlags; /* all the access flags that apply to fields fit into the first byte */
    NameType key;
    LOADED_CLASS_DEF pOwningClass;
} fieldDefStruct, *FIELD_DEF;


typedef struct constantFieldDefStruct {
    UINT16 fieldIndex; 
    UINT16 accessFlags;
    NameType key;
    LOADED_CLASS_DEF pOwningClass;
    jstackField value;
    jstackField value2;
} constantFieldDefStruct, *CONSTANT_FIELD_DEF;


#define getFieldDefSize() sizeof(fieldDefStruct)
#define getConstantFieldDefSize() sizeof(constantFieldDefStruct)
#define getFieldKey(pField) ((pField)->key.nameTypeKey)
#define isFinalField(pField) ((pField)->accessFlags & ACC_FINAL)
#define isPrimitiveField(pField) ((pField)->accessFlags & ACC_PRIMITIVE)
#define isDoubleField(pField) ((pField)->accessFlags & ACC_DOUBLE)
#define isConstantField(pField) ((pField)->accessFlags & ACC_INIT)

typedef struct throwableHandlerStruct {
    UINT16 startPCIndex;
    UINT16 endPCIndex;
    UINT16 handlerPCIndex;
    NamePackage catchKey;
} throwableHandlerStruct, *THROWABLE_HANDLER;


typedef struct throwableHandlerListStruct {
    UINT16 length;
    THROWABLE_HANDLER ppHandlers[1];
} throwableHandlerListStruct, *THROWABLE_HANDLER_LIST;

#define getHandlerListSize(n) (structOffsetOf(throwableHandlerListStruct, ppHandlers) + (n) * sizeof(THROWABLE_HANDLER))


struct commonMethodDefStruct {
    NameType nameType;
    UINT16 parameterVarCount; /* doubles and longs count as two, includes the instance for instance methods */
    LOADED_CLASS_DEF pOwningClass;
    UINT16 methodIndex; /* index into the method table */
};


struct commonClassMethodStruct {
    UINT16 accessFlags;
};


typedef struct methodDefStruct {
    struct commonMethodDefStruct base;
    struct commonClassMethodStruct classBase;

    INSTRUCTION pCode;
    UINT16 varCount; /* parameters and local variables, with doubles and floats counting as two */
    UINT16 localVarCount;
    UINT32 invokeCount; /* for stack chunking and checking stack limits, total size of a frame including operand stack, registers, locals and parameters */
    UINT32 byteCodeCount; /* number of bytes in the byte code */
    THROWABLE_HANDLER_LIST throwableHandlers;
    UINT16 maxStackWithMonitor;
    UINT32 *pParameterMap;

    /* will need members for verifier */
        
} methodDefStruct, *METHOD_DEF;

/* interface methods are implicitly public and abstract, and cannot be final, native, synchronized, strictfp, static, private or protected */
typedef struct interfaceMethodDefStruct {
    struct commonMethodDefStruct base;
} interfaceMethodDefStruct, *INTERFACE_METHOD_DEF;

typedef struct abstractMethodDefStruct {
    struct commonMethodDefStruct base;
    struct commonClassMethodStruct classBase;
} abstractMethodDefStruct, *ABSTRACT_METHOD_DEF;


typedef struct nativeMethodDefStruct {
    struct commonMethodDefStruct base;
    struct commonClassMethodStruct classBase;
    UINT16 nativeFunctionIndex;
} nativeMethodDefStruct, *NATIVE_METHOD_DEF;


#define PARAMETER_MAP_INDEX_SHIFT 5
#define PARAMETER_MAP_REMAINDER_MASK 0x1f

#define getInterfaceMethodSize() (sizeof(interfaceMethodDefStruct))
#define getClassMethodSize() (sizeof(methodDefStruct))
#define getAbstractMethodSize() (sizeof(abstractMethodDefStruct))
#define getNativeMethodSize() (sizeof(nativeMethodDefStruct))
#define isNativeMethod(pMethod) ((pMethod)->classBase.accessFlags & ACC_NATIVE)
#define isInitializerMethod(pMethod) ((pMethod)->classBase.accessFlags & ACC_INIT)
#define isInitializerOrPrivateMethod(pMethod) ((pMethod)->classBase.accessFlags & ACC_INIT_PRIVATE)
#define isPublicMethod(pMethod) ((pMethod)->classBase.accessFlags & ACC_PUBLIC)
#define isPrivateMethod(pMethod) ((pMethod)->classBase.accessFlags & ACC_PRIVATE)
#define isSynchronizedMethod(pMethod) ((pMethod)->classBase.accessFlags & ACC_SYNCHRONIZED)
#define isAbstractMethod(pMethod) ((pMethod)->classBase.accessFlags & ACC_ABSTRACT)
#define isStaticMethod(pMethod) ((pMethod)->classBase.accessFlags & ACC_STATIC)
#define getMethodNameKey(pMethod) ((pMethod)->base.nameType.nt.nameKey)
#define getMethodTypeKey(pMethod) ((pMethod)->base.nameType.nt.typeKey)
#define getMethodKey(pMethod) ((pMethod)->base.nameType.nameTypeKey)
#define setNativeMethodLinked(pNativeMethod) ((pNativeMethod)->classBase.accessFlags |= ACC_NATIVE_LINKED)
#define isNativeMethodLinked(pNativeMethod) ((pNativeMethod)->classBase.accessFlags & ACC_NATIVE_LINKED)
#define setMethodFiltered(pMethod) ((pMethod)->classBase.accessFlags |= ACC_FILTERED)
#define isMethodFiltered(pMethod) ((pMethod)->classBase.accessFlags & ACC_FILTERED)


/* fields in a class' field lists are ordered as follows:
    instanceFieldList:
        1 - object
        2 - primitive
    staticFieldList:
        3 - object
        4 - primitive

In the class structure, we keep counts of 1, 1-2, 3, 3-4.

*/


typedef struct fieldListStruct {
    UINT16 length;
    FIELD_DEF ppFields[1];
} fieldListStruct, *FIELD_LIST;

#define getFieldListSize(n) (structOffsetOf(fieldListStruct, ppFields) + (n) * sizeof(FIELD_DEF))
#define getFieldDef(pList, index) ((pList)->ppFields[index])


/* methods in a class are ordered as follows:
    instanceMethodList:
        1 - virtual (non-private non-constructor non-static methods)
        2 - final virtual (non-private non-constructor non-static methods declared final)
        3 - private (non-static non-constructor)
        4 - <init>s (constructors)
    staticMethodList:
        5 - statics (all statics excluding clinit)
        6 - <clinit>

Invokevirtual searches 1 and 2.  
Invokeinterface searches through the interface implementation table which is populated by 1 and 2.
Invokespecial (inits, privates, supers) searches 1, 2, 3, 4.
Invokestatic searches 5.

In the class structure, we keep counts of 1-2, 1-4, 5, and a bit in the access flags determines if there is 6.

Only categories 1 and 2 are inherited from a superclass.  Because of quickening (ie an index to
a virtual method must be the same in all child classes), categories 1 and 2 in a child class actually end up looking like:
    1 - parent class methods - a mix of virtual and final virtual
    2 - virtual
    3 - final virtual

*/

typedef struct methodListStruct {
    UINT16 length;
    METHOD_DEF ppMethods[1];
} methodListStruct, *METHOD_LIST;

#define getMethodListSize(n) (structOffsetOf(methodListStruct, ppMethods) + (n) * sizeof(METHOD_DEF))
#define getMethod(pList, index) ((pList)->ppMethods[index])



typedef struct interfaceListStruct {
    UINT16 length;

     /* Given the index i2 of the method in interface i1 in the list below, the method
      * entry for the method is given as: interfaceMethodTable[i1][i2] 
      */
    METHOD_DEF **pppInterfaceImplementationTable;
    LOADED_INTERFACE_DEF pInterfaceDefs[1]; 
} interfaceListStruct, *INTERFACE_LIST;

#define getInterfaceListSize(n) (structOffsetOf(struct interfaceListStruct, pInterfaceDefs) + (n) * sizeof(LOADED_INTERFACE_DEF))


#endif
