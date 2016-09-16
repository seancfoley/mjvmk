#ifndef PRELOAD_H
#define PRELOAD_H

#include "elements/base.h"
#include "interpret/returnCodes.h"
#include "string/javaString.h"
#include "resolution/constantPool.h"
#include "resolution/file/fileAccess.h"
#include "iterativeLoad.h"


typedef struct constantUtf8StringStruct {
    UINT16 length;
    char utf8String[];
} constantUtf8StringStruct, *CONSTANT_UTF8_STRING;

#define getConstantStringSize(numChars) (structOffsetOf(constantUtf8StringStruct, utf8String) + numChars)

/* 
 * This structure represents an entry in the constant pool after the first pass, 
 * before it is in its final pre-runtime state.  The final pre-runtime constant pool will also have
 * its contents altered by run-time resolution.
 */
union firstPassConstantPoolEntryUnion
{
    UINT32 length;  /* length of the constant pool */    
    constantPoolMethodStruct method; /* CONSTANT_Methodref, CONSTANT_InterfaceMethodref */   
    constantPoolFieldStruct field; /* CONSTANT_Fieldref */
    UINT16 index; /* CONSTANT_Class, CONSTANT_String */
    jstackField constantValue; /* CONSTANT_Integer, CONSTANT_Float, CONSTANT_Long, CONSTANT_Double */
    NameType nameType; /* CONSTANT_NameAndType */
    CONSTANT_UTF8_STRING pUtf8String; /* CONSTANT_Utf8 */
};

/* the method parameter map is a bit map indicating which parameters are object types */
#define getMethodParameterMapSize(numArgs) (((numArgs) + 0x07) >> 3) /* this is the size in bytes */
void createMethodTypeParameterMap(const char *type, UINT16 typeLength, BOOLEAN isInstanceMethod, UINT16 mapArgs, UINT32 pMap[]);

BOOLEAN verifyName(const char *name, UINT16 length);
BOOLEAN verifyFieldType(const char* name, UINT16 length);
BOOLEAN verifyMethodType(const char* type, UINT16 typeLength, UINT16 *pNumArgs);
#define verifyMethodName(name, length) verifyName(name, length)
#define verifyInitializerMethodName(nameType, typeString, typeLength)                   \
    (((nameType).nameTypeKey == clinitNameType.nameTypeKey)                             \
        || (((nameType).nt.nameKey == initNameType.nt.nameKey) &&                       \
            ((typeString)[(typeLength) - 1] == VOID_RETURN_TYPE)))
#define verifyFieldName(name, length) verifyName(name, length)

/* 
 * macro will work on both the first and second pass constant pool, 
 * but the second pass pool could be missing some or all of its UTF8 string entries. 
 */
#define isInvalidConstantPoolEntry(pConstantPool, index, type)              \
    (index >= getConstantPoolLength((CONSTANT_POOL) pConstantPool)          \
        || type != getConstantPoolTag(pConstantPool, index))

RETURN_CODE preloadConstantPool(FILE_ACCESS pClassFile, CONSTANT_POOL *ppPool, FIRST_PASS_CONSTANT_POOL *ppOrigPool);
RETURN_CODE preloadClass(PRELOAD_DATA pPreloadData);


#endif

