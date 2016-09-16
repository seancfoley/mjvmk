#ifndef CONSTANTPOOL_H
#define CONSTANTPOOL_H

#include "elements/base.h"


/* 
 * possible entries in the class file constant pool and their intermediate and final mappings:  
 * These are not defined as enum because in many places we will store them as a single byte and not an int.
 */

#define CONSTANT_Utf8               1
#define CONSTANT_Integer            3
#define CONSTANT_Float              4
#define CONSTANT_Long               5
#define CONSTANT_Double             6
#define CONSTANT_Class              7
#define CONSTANT_String             8
#define CONSTANT_Fieldref           9
#define CONSTANT_Methodref          10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_FieldNameAndType   13
#define CONSTANT_MethodNameAndType  14


/* the following type of constant is not available at run-time, only load-time */
#define CONSTANT_NameAndType 12

typedef UINT8 ConstantPoolTag;

/*  Cache bit - If the tag includes the cache bit, then the constant pool
 *  entry is fully resolved
 */

#define CP_CACHEBIT  0x80
#define CP_CACHEMASK 0x7F


typedef struct constantPoolMethodStruct {
    UINT16 classIndex;
    UINT16 nameTypeIndex;
} constantPoolMethodStruct, *CONSTANT_POOL_METHOD;

typedef constantPoolMethodStruct constantPoolFieldStruct, *CONSTANT_POOL_FIELD;
typedef constantPoolMethodStruct constantPoolInterfaceMethodStruct, *CONSTANT_POOL_INTERFACE_METHOD;


union constantPoolEntryUnion {
    UINT32 length;                                      /* length of the constant pool */   
    jstackField constantValue;                          /* java.lang.String and number constants */
    
    /* constants that can be further resolved after constant pool loading */

    constantPoolMethodStruct method;                    /* CONSTANT_Methodref */
    constantPoolInterfaceMethodStruct interfaceMethod;  /* CONSTANT_InterfaceMethodref */
    constantPoolFieldStruct field;                      /* CONSTANT_Fieldref */
    NameType nameType;                                  /* CONSTANT_FieldNameAndType, CONSTANT_MethodNameAndType referred to by fields, methods, interface methods */
    NamePackage namePackage;                            /* CONSTANT_Class, not yet loaded */


    /* Cached entries, resolved after constant pool loading.  
     * If the tag has the cache bit set, we've got one of the following.
     */
    FIELD_DEF pFieldDef;                                /* substitutes field for CONSTANT_Fieldref */
    METHOD_DEF pMethodDef;                              /* substitutes method for CONSTANT_Methodref */
    INTERFACE_METHOD_DEF pInterfaceMethodDef;           /* substitutes interfaceMethod for CONSTANT_InterfaceMethodref */
    COMMON_CLASS_DEF pClassDef;                         /* substitutes class for CONSTANT_Class */
};





#define getConstantPoolLength(pConstantPool)                                        ((pConstantPool)[0].length)
#define getConstantPoolTags(pConstantPool)                                          ((UINT8 *) (&(pConstantPool)[getConstantPoolLength(pConstantPool)]))
#define getConstantPoolTag(pConstantPool, index)                                    (getConstantPoolTags(pConstantPool)[index])
#define getConstantPoolUnmaskedTag(pConstantPool, index)                            (getConstantPoolTags(pConstantPool)[index] & CP_CACHEMASK)
#define getConstantPoolSize(length)                                                 ((sizeof(constantPoolEntryUnion) + sizeof(UINT8)) * (length))

#define indicateConstantPoolEntryIsResolved(pConstantPool, index)                   (getConstantPoolTag(pConstantPool, index) |= CP_CACHEBIT)
#define indicateConstantPoolClassIsResolved(pConstantPool, index)                   indicateConstantPoolEntryIsResolved(pConstantPool, index)
#define indicateConstantPoolFieldIsResolved(pConstantPool, index)                   indicateConstantPoolEntryIsResolved(pConstantPool, index)
#define indicateConstantPoolMethodIsResolved(pConstantPool, index)                  indicateConstantPoolEntryIsResolved(pConstantPool, index)
#define indicateConstantPoolInterfaceMethodIsResolved(pConstantPool, index)         indicateConstantPoolEntryIsResolved(pConstantPool, index)

#if RUN_TIME_CONSTANT_POOL_RESOLUTION
#define constantPoolEntryIsResolved(pConstantPool, index)                           (getConstantPoolTag(pConstantPool, index) & CP_CACHEBIT)
#define constantPoolClassIsResolved(pConstantPool, index)                           constantPoolEntryIsResolved(pConstantPool, index)
#define constantPoolMethodIsResolved(pConstantPool, index)                          constantPoolEntryIsResolved(pConstantPool, index)
#define constantPoolFieldIsResolved(pConstantPool, index)                           constantPoolEntryIsResolved(pConstantPool, index)
#define constantPoolInterfaceMethodIsResolved(pConstantPool, index)                 constantPoolEntryIsResolved(pConstantPool, index)
#else
/* if you know everything is resolved already (absolutely everything was romized), then the following should all be TRUE */
#define constantPoolEntryIsResolved(pConstantPool, index)                           FALSE
#define constantPoolClassIsResolved(pConstantPool, index)                           FALSE
#define constantPoolMethodIsResolved(pConstantPool, index)                          FALSE
#define constantPoolFieldIsResolved(pConstantPool, index)                           FALSE
#define constantPoolInterfaceMethodIsResolved(pConstantPool, index)                 FALSE
#endif


#define getConstantPoolEntry(pConstantPool, index)                                  (&(pConstantPool)[index])

/* entries that are never resolved during runtime */
#define getConstantPoolInternedString(pConstantPool, index)                         (getInternedStringInstanceFromObjectInstance(extractStackFieldObject((pConstantPool)[index].constantValue)))
#define getConstantPoolInteger(pConstantPool, index)                                (extractStackFieldInt((pConstantPool)[index].constantValue))
#define getConstantPoolLong(pConstantPool, index)                                   (extractStackFieldLong((pConstantPool)[index].constantValue)) /* returns JLONG */
#define getConstantPoolValue(pConstantPool, index)                                  ((pConstantPool)[index].constantValue)
#define getConstantPoolNameType(pConstantPool, index)                               ((pConstantPool)[index].nameType)
#if IMPLEMENTS_FLOAT
#define getConstantPoolFloat(pConstantPool, index)                                  (extractStackFieldFloat((pConstantPool)[index].constantValue))
#define getConstantPoolDouble(pConstantPool, index)                                 (extractStackFieldDouble((pConstantPool)[index].constantValue))
#endif

/* these macros get you what you want whether the entry is run-time resolved or not */
#define getConstantPoolClassDef(pConstantPool, index)                               (constantPoolClassIsResolved(pConstantPool, index) ? (pConstantPool)[index].pClassDef : getClassDefInTable(getConstantPoolNamePackage(pConstantPool, index).namePackageKey))                                            
#define getConstantPoolNamePackage(pConstantPool, index)                            (constantPoolClassIsResolved(pConstantPool, index) ? (pConstantPool)[index].pClassDef->key : (pConstantPool)[index].namePackage)

/* entries not yet runtime resolved */
#define getConstantPoolField(pConstantPool, index)                                  (FIRST_ASSERT(!constantPoolFieldIsResolved(pConstantPool, index)) &(pConstantPool)[index].field)
#define getConstantPoolMethod(pConstantPool, index)                                 (FIRST_ASSERT(!constantPoolMethodIsResolved(pConstantPool, index)) &(pConstantPool)[index].method)
#define getConstantPoolInterfaceMethod(pConstantPool, index)                        (FIRST_ASSERT(!constantPoolInterfaceMethodIsResolved(pConstantPool, index)) &(pConstantPool)[index].interfaceMethod)

/* entries that are runtime resolved */
#define getConstantPoolFieldDef(pConstantPool, index)                               (FIRST_ASSERT(constantPoolFieldIsResolved(pConstantPool, index)) (pConstantPool)[index].pFieldDef)
#define getConstantPoolMethodDef(pConstantPool, index)                              (FIRST_ASSERT(constantPoolMethodIsResolved(pConstantPool, index)) (pConstantPool)[index].pMethodDef)
#define getConstantPoolInterfaceMethodDef(pConstantPool, index)                     (FIRST_ASSERT(constantPoolInterfaceMethodIsResolved(pConstantPool, index)) (pConstantPool)[index].pInterfaceMethodDef)

/* runtime resolution */
#define setConstantPoolClassDef(pConstantPool, index, pClassDef)                    ((pConstantPool)[index].pClassDef = (pClassDef))
#define setConstantPoolFieldDef(pConstantPool, index, pField)                       ((pConstantPool)[index].pFieldDef = (pField))
#define setConstantPoolMethodDef(pConstantPool, index, pMethod)                     ((pConstantPool)[index].pMethodDef = (pMethod))
#define setConstantPoolInterfaceMethodDef(pConstantPool, index, pInterfaceMethod)   ((pConstantPool)[index].pInterfaceMethodDef = (pInterfaceMethod))




#endif

