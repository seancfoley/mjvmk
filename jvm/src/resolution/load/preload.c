

#include "memory/memoryAlloc.h"
#include "string/stringKeys.h"
#include "log/logItem.h"
#include "preload.h"


BOOLEAN verifyName(const char *name, UINT16 length)
{
    UINT8 charLength;
    const char *str;
    const char *end;
    char c;

    if(length == 0) {
        return FALSE;
    }
    c = *name;
    if(c >= '0' && c <= '9') {
        return FALSE;
    }
    str = name;
    end = name + length;
    while(str < end) {
        if(c < 127) {
            if((c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') ||
               (c == '_') ||
               (c == '$')) 
            {
                str++;
            }
            else {
                return FALSE;
            }
        }
        else {
            /* whatever unicode character it is, we'll assume it's OK */
            getUnicodeCharFromUTF8Char(str, &charLength);
            str += charLength;
        }
        c = *str;
    }
    return TRUE;
}

static const char *skipName(const char *name, UINT16 totalLength, BOOLEAN *reachedEnd)
{
    UINT8 charLength;
    const char *str;
    const char *end;
    char c = *name;

    if(c >= '0' && c <= '9') {
        return name;
    }
    str = name;
    end = name + totalLength;
    while(str < end) {
        if(c < 127) {
            if((c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') ||
               (c == '_') ||
               (c == '$')) 
            {
                str++;
            }
            else {
                *reachedEnd = FALSE;
                return str;
            }
        }
        else {
            /* whatever unicode character it is, we'll assume it's OK */
            getUnicodeCharFromUTF8Char(str, &charLength);
            str += charLength;
        }
        c = *str;
    }
    *reachedEnd = TRUE;
    return str;
}


static const char *skipClassName(const char *name, UINT16 totalLength)
{
    const char *str = name;
    BOOLEAN reachedEnd;
    
    while(TRUE) {
        if(*str == '/') {
            return str;
        }
        str = skipName(str, totalLength, &reachedEnd);
        if(reachedEnd || *str != '/') {
            return str;
        }
        str++;
    }
}


static const char *skipType(const char* name, UINT16 totalLength)
{
    const char *str;
    
    if(totalLength <= 2) {
        return NULL;
    }
    switch(*name) {
        case BOOLEAN_FIELD_TYPE:            
        case CHAR_FIELD_TYPE:            
        case INT_FIELD_TYPE:            
        case LONG_FIELD_TYPE:
        case SHORT_FIELD_TYPE:
        case BYTE_FIELD_TYPE:
#if IMPLEMENTS_FLOAT
        case FLOAT_FIELD_TYPE:
        case DOUBLE_FIELD_TYPE:
#endif
            return name + 1;

        case OBJECT_FIELD_TYPE:
            str = skipClassName(name + 1, (UINT16) (totalLength - 1));
            /* there must be a ';', as well as room for the terminating ')' and room for the return type */
            if((str - name > totalLength - 3) || *str != ';') {
                return NULL;
            }
            return str + 1;

        case ARRAY_FIELD_TYPE: {
            UINT16 dimensions;
            const char *end = name + totalLength;
            str = name;
            do {
                str++;
            } while(str < end && *str == ARRAY_FIELD_TYPE);
            dimensions = str - name;
            if(dimensions > 255) { /* 255 dimensions is the maximum allowed, dictated by the VM spec */
                return NULL;
            }
            return skipType(str, (UINT16) (totalLength - dimensions));
        }
        default:
            return NULL;
    }
    return NULL;
}

static BOOLEAN verifyLoadedClassName(const char *name, UINT16 length)
{
    const char *str;
    const char *previousStr = name;
    BOOLEAN reachedEnd;
    
    while(TRUE) {
        str = skipName(previousStr, length, &reachedEnd);
        if(str == previousStr) {
            return FALSE;
        }
        if(reachedEnd) {
            return TRUE;
        }
        if(*str != '/') {
            return FALSE;
        }
        length -= (str - previousStr) + 1;
        previousStr = str + 1;
    }
    /* will never reach here */
    return FALSE;
}

BOOLEAN verifyFieldType(const char* name, UINT16 length)
{
    const char *str;
    
    if(length == 0) {
        return FALSE;
    }
    switch(*name) {
        case BOOLEAN_FIELD_TYPE:            
        case CHAR_FIELD_TYPE:            
        case INT_FIELD_TYPE:            
        case LONG_FIELD_TYPE:
        case SHORT_FIELD_TYPE:
        case BYTE_FIELD_TYPE:
#if IMPLEMENTS_FLOAT
        case FLOAT_FIELD_TYPE:
        case DOUBLE_FIELD_TYPE:
#endif
            return length == 1;

        case OBJECT_FIELD_TYPE:
            return length > 2 && name[length - 1] == ';' && verifyLoadedClassName(name + 1, (UINT16) (length - 2));

        case ARRAY_FIELD_TYPE: {
            UINT16 dimensions;
            const char *end = name + length;
            str = name;
            do {
                str++;
            } while(str < end && *str == ARRAY_FIELD_TYPE);
            dimensions = str - name;
            if(dimensions > 255) { /* 255 dimensions is the maximum allowed, dictated by the VM spec */
                return FALSE;
            }
            return verifyFieldType(str, (UINT16) (length - dimensions));
        }

        default:
            return FALSE;
    }
}

/* creates a bit-map indicating which parameters are objects as opposed to primitives - longs and 
 * doubles each count as two primitive parameters.
 */
void createMethodTypeParameterMap(const char *type, UINT16 typeLength, BOOLEAN isInstanceMethod, UINT16 mapArgs, UINT32 pMap[])
{
    UINT16 index;
    const char *str = type + 1;

    for(index = 0; index < mapArgs; index++) {
        pMap[index] = 0;
    }

    if(isInstanceMethod) {
        pMap[0] = 1;
        index = 1;
    }
    else {
        index = 0;
    }

    do {
        switch(*str) {
            case LONG_FIELD_TYPE:
#if IMPLEMENTS_FLOAT
            case DOUBLE_FIELD_TYPE:
#endif
                str++;
                index += 2;
                continue;

            case BOOLEAN_FIELD_TYPE:            
            case CHAR_FIELD_TYPE:            
            case INT_FIELD_TYPE:            
            case SHORT_FIELD_TYPE:
            case BYTE_FIELD_TYPE:
#if IMPLEMENTS_FLOAT
            case FLOAT_FIELD_TYPE:
#endif
                str++;
                index++;
                continue;

            case OBJECT_FIELD_TYPE:
                str = skipClassName(str + 1, (UINT16) (typeLength - (str - type))) + 1;
                pMap[index >> PARAMETER_MAP_INDEX_SHIFT] |= (1 << (index & PARAMETER_MAP_REMAINDER_MASK));
                index++;
                continue;

            case ARRAY_FIELD_TYPE:
                do {
                    str++;
                } while(*str == ARRAY_FIELD_TYPE);
                str = skipType(str, (UINT16) (typeLength - (str - type)));
                pMap[index >> PARAMETER_MAP_INDEX_SHIFT] |= (1 << (index & PARAMETER_MAP_REMAINDER_MASK));
                index++;
                continue;

            case ')':
                return;
        }
    } while(TRUE);
}

BOOLEAN verifyMethodType(const char* type, UINT16 typeLength, UINT16 *pNumArgs)
{
    const char *str;
    const char *previousStr;
    UINT16 numArgs;

    if(typeLength == 0) {
        return FALSE;
    }
    if(*type != '(') {
        return FALSE;
    }
    numArgs = 0;
    previousStr = type + 1;
    typeLength--;
    while(TRUE) {
        str = skipType(previousStr, typeLength);
        if(str == NULL) {
            previousStr;
            /* there should be a terminating ')' */
            if(*previousStr != ')') {
                return FALSE;
            }
            /* record the number of parameters indicated by the type string */
            if(pNumArgs) {
                *pNumArgs = numArgs;
            }
            /* check for a valid return type */
            ++previousStr;
            if(*previousStr == VOID_RETURN_TYPE) {
                return typeLength == 2;  
            }
            else {
                return verifyFieldType(previousStr, (UINT16) (typeLength - 1));
            }
        }
        if(*previousStr == LONG_FIELD_TYPE
#if IMPLEMENTS_FLOAT
            || *previousStr == DOUBLE_FIELD_TYPE
#endif
        ) {
            numArgs += 2;
        }
        else {
            numArgs++;
        }
        typeLength -= str - previousStr;
        previousStr = str;
    }
    return FALSE;
}

static BOOLEAN verifyClassName(const char *name, UINT16 length) 
{
    if(length == 0) {
        return FALSE;
    }
    if(*name == '[') {
        return verifyFieldType(name + 1, (UINT16) (length - 1));
    }
    return verifyLoadedClassName(name, length);
}



static void cleanUpFirstPool(FIRST_PASS_CONSTANT_POOL_ENTRY firstPool, UINT8 *tags, UINT32 highestIndex) 
{
    for(; highestIndex > 0; highestIndex--) {
        if(tags[highestIndex] == CONSTANT_Utf8 && firstPool[highestIndex].pUtf8String != NULL) {
            memoryFree(firstPool[highestIndex].pUtf8String);
        }
    }
    memoryFree(firstPool);
}


static RETURN_CODE preloadConstantPool(FILE_ACCESS pClassFile, CONSTANT_POOL *ppPool, FIRST_PASS_CONSTANT_POOL *ppOrigPool)
{
    UINT16 constantCount;
    CONSTANT_POOL_ENTRY secondPassPool;
    UINT32 cpIndex;
    RETURN_CODE ret = SUCCESS;
    FIRST_PASS_CONSTANT_POOL_ENTRY firstPassPool = NULL;
    UINT8 tag;
    UINT8 *firstPassConstantPooltags;
    UINT8 *secondPassConstantPoolTags;
    UINT32 lastNonUtfIndex = -1;
           
    /* for pass 1 */
    UINT8 (*byteLoader) (FILE_ACCESS) = pClassFile->pFileAccessMethods->loadByte;
    UINT16 (*u2Loader) (FILE_ACCESS) = pClassFile->pFileAccessMethods->loadU2;
    UINT32 (*u4Loader) (FILE_ACCESS) = pClassFile->pFileAccessMethods->loadU4;
    UINT32 (*bytesLoader) (FILE_ACCESS, UINT32, UINT8 *) = pClassFile->pFileAccessMethods->loadBytes;
    CONSTANT_UTF8_STRING pUtf8String;
    jint intValue;
    jlong longValue;
    
    /* for pass 2 */
    UINT16 index, index2, index3;
    INTERNED_STRING_INSTANCE internedString;
    CONSTANT_UTF8_STRING pUtf8String2;
    UTF8_STRING pUtf8PoolString;
    UTF8_TYPE_STRING pUtf8TypePoolString;
    classIdentifierStruct classID;
    NameType nameType;
    
    jumpBufferStruct errorJump;

    pClassFile->pErrorJump = &errorJump;

    /* we'll jump back here if there is a file read error */
    if(setJump(errorJump) != 0) {
        if(firstPassPool != NULL) {
            cleanUpFirstPool(firstPassPool, firstPassConstantPooltags, constantCount - 1);
        }
        return pClassFile->errorFlag;
    }
    constantCount = u2Loader(pClassFile);

    /* allocate memory for the entries and the tags */

    firstPassPool = memoryCalloc(getConstantPoolSize(constantCount), 1);
    if(firstPassPool == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    firstPassConstantPooltags = (UINT8 *) (firstPassPool + constantCount);
    firstPassPool[0].length = constantCount;
    
    /*  Read each constant pool entry in the class file */
    for (cpIndex = 1; cpIndex < constantCount; cpIndex++) {
    
        switch (firstPassConstantPooltags[cpIndex] = byteLoader(pClassFile)) {

            case CONSTANT_String: 
            case CONSTANT_Class: 
                /* points to a UTF string */
                firstPassPool[cpIndex].index = u2Loader(pClassFile);
                break;

            case CONSTANT_Fieldref: 
                /* fields are handled exactly the same way as methods; just fall through */

            case CONSTANT_Methodref: 
            case CONSTANT_InterfaceMethodref:
                firstPassPool[cpIndex].method.classIndex = u2Loader(pClassFile);
                firstPassPool[cpIndex].method.nameTypeIndex = u2Loader(pClassFile);
                break;

            case CONSTANT_Integer: 
                insertStackFieldInt(firstPassPool[cpIndex].constantValue, u4Loader(pClassFile));
                break;

            case CONSTANT_Long:
                if (cpIndex + 1 >= constantCount) {
                    cleanUpFirstPool(firstPassPool, firstPassConstantPooltags, cpIndex);
                    return ERROR_CODE_INCOMPLETE_CONSTANT_POOL_ENTRY;
                }
                intValue = u4Loader(pClassFile);
                setJlong(longValue, intValue, u4Loader(pClassFile));
                insertStackFieldLong(firstPassPool[cpIndex].constantValue, longValue);
                cpIndex++;
                break;

#if IMPLEMENTS_FLOAT
            case CONSTANT_Float:
                insertStackFieldFloat(firstPassPool[cpIndex].constantValue, (jfloat) u4Loader(pClassFile));
                break;

            case CONSTANT_Double:
                if (cpIndex + 1 >= constantCount) {
                    cleanUpFirstPool(firstPassPool, firstPassConstantPooltags, cpIndex);
                    return ERROR_CODE_INCOMPLETE_CONSTANT_POOL_ENTRY;
                }
                intValue = u4Loader(pClassFile);
                setJlong(longValue, intValue, u4Loader(pClassFile));
                insertStackFieldDouble(firstPassPool[cpIndex].constantValue, *(JDOUBLE) &longValue);
                cpIndex++;
                break;
#endif
            case CONSTANT_NameAndType:
                /* used by fields and methods for name and type info */
                firstPassPool[cpIndex].nameType.nt.nameKey = u2Loader(pClassFile);
                firstPassPool[cpIndex].nameType.nt.typeKey = u2Loader(pClassFile);
                break;

            case CONSTANT_Utf8:
                index = u2Loader(pClassFile);
                pUtf8String = memoryAlloc(getConstantStringSize(index));
                if(pUtf8String == NULL) {
                    cleanUpFirstPool(firstPassPool, firstPassConstantPooltags, cpIndex);
                    return ERROR_CODE_OUT_OF_MEMORY;
                }
                firstPassPool[cpIndex].pUtf8String = pUtf8String;
                pUtf8String->length = index;
                bytesLoader(pClassFile, index, pUtf8String->utf8String);
                if(!verifyUTF8String(pUtf8String->utf8String, index)) {
                    cleanUpFirstPool(firstPassPool, firstPassConstantPooltags, cpIndex);
                    return ERROR_CODE_INVALID_UTF8_STRING;
                }
                
                break;
            default: 
                cleanUpFirstPool(firstPassPool, firstPassConstantPooltags, cpIndex);
                return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
        }
    }
    pClassFile->pErrorJump = NULL;

    for(lastNonUtfIndex = constantCount - 1; lastNonUtfIndex >= 0 && firstPassConstantPooltags[lastNonUtfIndex] == CONSTANT_Utf8; lastNonUtfIndex--);
    secondPassPool = memoryCalloc(getConstantPoolSize(lastNonUtfIndex + 1), 1);
    if(secondPassPool == NULL) {
        cleanUpFirstPool(firstPassPool, firstPassConstantPooltags, constantCount - 1);
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    secondPassPool[0].length = lastNonUtfIndex + 1;
    secondPassConstantPoolTags = getConstantPoolTags(secondPassPool);

    
    
    
    /* Now create our constant pool entries */
    for (cpIndex = 1; cpIndex <= lastNonUtfIndex; cpIndex++) { 

        tag = firstPassConstantPooltags[cpIndex];
        
        switch (tag) {

            case CONSTANT_Class:
                /* resolve to a class instance */

                index = firstPassPool[cpIndex].index;
                pUtf8String = firstPassPool[index].pUtf8String;

                if(!verifyClassName(pUtf8String->utf8String, pUtf8String->length)) {
                    ret = ERROR_CODE_INVALID_NAME;
                    break;
                }
                if((ret = getClassParams(pUtf8String->utf8String, pUtf8String->length, &classID)) != SUCCESS) {
                    break;
                }
                
                secondPassPool[cpIndex].namePackage = classID.key;  /* ANSI union assignment */
                secondPassConstantPoolTags[cpIndex] = CONSTANT_Class;
                break;
        
            case CONSTANT_String: 
                
                /* resolve to an interned string */

                index = firstPassPool[cpIndex].index;
                pUtf8String = firstPassPool[index].pUtf8String;
                if(SUCCESS != (ret = internString(pUtf8String->utf8String, pUtf8String->length, &internedString))) {
                    break;
                }
                insertStackFieldObject(secondPassPool[cpIndex].constantValue, (OBJECT_INSTANCE) &internedString->header);
                secondPassConstantPoolTags[cpIndex] = CONSTANT_String;
                break;
        

            case CONSTANT_Fieldref: 

                /* just verification */

                index = firstPassPool[cpIndex].field.classIndex;
                index2 = firstPassPool[cpIndex].field.nameTypeIndex;
                if (index >= constantCount ||
                    index2 >= constantCount ||
                    firstPassConstantPooltags[index] != CONSTANT_Class ||
                    firstPassConstantPooltags[index2] != CONSTANT_NameAndType) {

                    ret = ERROR_CODE_INVALID_MEMBER_REF;
                    break;
                }
                index3 = firstPassPool[index2].nameType.nt.typeKey;
                pUtf8String = firstPassPool[index3].pUtf8String;

                if (pUtf8String->utf8String[0] == '(') {
                    ret = ERROR_CODE_INVALID_MEMBER_REF;
                    break;
                }
                secondPassPool[cpIndex].field = firstPassPool[cpIndex].field;
                secondPassConstantPoolTags[cpIndex] = CONSTANT_Fieldref;
                break;

            case CONSTANT_Methodref: 
            case CONSTANT_InterfaceMethodref:

                /* just verification */

                index = firstPassPool[cpIndex].method.classIndex;
                index2 = firstPassPool[cpIndex].method.nameTypeIndex;
                if (index >= constantCount ||
                    index2 >= constantCount ||
                    firstPassConstantPooltags[index] != CONSTANT_Class ||
                    firstPassConstantPooltags[index2] != CONSTANT_NameAndType) {

                    ret = ERROR_CODE_INVALID_MEMBER_REF;
                    break;
                } 
                secondPassPool[cpIndex].method = firstPassPool[cpIndex].method;
                secondPassConstantPoolTags[cpIndex] = firstPassConstantPooltags[cpIndex];
                break;

#if IMPLEMENTS_FLOAT
            case CONSTANT_Double:
#endif
            case CONSTANT_Long: 
                secondPassPool[cpIndex].constantValue = firstPassPool[cpIndex].constantValue;
                secondPassConstantPoolTags[cpIndex] = firstPassConstantPooltags[cpIndex];
                cpIndex++; 
                secondPassPool[cpIndex].constantValue = firstPassPool[cpIndex].constantValue;
                secondPassConstantPoolTags[cpIndex] = 0;
                break;

#if IMPLEMENTS_FLOAT
            case CONSTANT_Float:
#endif
            case CONSTANT_Integer: 
                secondPassPool[cpIndex].constantValue = firstPassPool[cpIndex].constantValue;
                secondPassConstantPoolTags[cpIndex] = firstPassConstantPooltags[cpIndex];
                break;

            case CONSTANT_NameAndType:
                nameType = firstPassPool[cpIndex].nameType;
                index = nameType.nt.nameKey;
                index2 = nameType.nt.typeKey;
                pUtf8String = firstPassPool[index].pUtf8String;
                pUtf8String2 = firstPassPool[index2].pUtf8String;

                if(pUtf8String2->length == 0 || pUtf8String->length == 0) {
                    ret = ERROR_CODE_INVALID_NAME;
                    break;
                }

                if (pUtf8String2->utf8String[0] == '(') {
                    if (pUtf8String->utf8String[0] == '<') {
                        if(pUtf8String->length >= 6 && pUtf8String->utf8String[pUtf8String->length - 1] == '>') {
                            if(!verifyMethodName(pUtf8String->utf8String + 1, (UINT16) (pUtf8String->length - 2))) {
                                ret = ERROR_CODE_INVALID_NAME;
                                break;
                            }
                        }
                        else {
                            ret = ERROR_CODE_INVALID_NAME;
                            break;
                        }
                    }
                    else if(!verifyMethodName(pUtf8String->utf8String, pUtf8String->length)) {
                        ret = ERROR_CODE_INVALID_NAME;
                        break;
                    }
                    if(!verifyMethodType(pUtf8String2->utf8String, pUtf8String2->length, NULL)) {
                        ret = ERROR_CODE_INVALID_METHOD_TYPE;
                        break;
                    }
                    if(SUCCESS != (ret = getMethodTypeString(pUtf8String2->utf8String, pUtf8String2->length, &pUtf8TypePoolString))) {
                        break;
                    }
                    secondPassConstantPoolTags[cpIndex] = CONSTANT_MethodNameAndType;
                } 
                else {
                    if(!verifyFieldName(pUtf8String->utf8String, pUtf8String->length)) {
                        ret = ERROR_CODE_INVALID_NAME;
                        break;
                    }
                    if(!verifyFieldType(pUtf8String2->utf8String, pUtf8String2->length)) {
                        ret = ERROR_CODE_INVALID_FIELD_TYPE;
                        break;
                    }
                    if(SUCCESS != (ret = getFieldTypeString(pUtf8String2->utf8String, pUtf8String2->length, &pUtf8TypePoolString))) {
                        break;
                    }
                    secondPassConstantPoolTags[cpIndex] = CONSTANT_FieldNameAndType;
                }
                secondPassPool[cpIndex].nameType.nt.typeKey = pUtf8TypePoolString->typeKey;
                
                if((ret = getUTF8StringByChars(pUtf8String->utf8String, pUtf8String->length, &pUtf8PoolString)) != SUCCESS) {
                    break;
                }
                secondPassPool[cpIndex].nameType.nt.nameKey = pUtf8PoolString->nameKey;

                if (pUtf8String->utf8String[0] == '<') {
                    if(!verifyInitializerMethodName(secondPassPool[cpIndex].nameType, pUtf8String2->utf8String, pUtf8String2->length)) {
                        ret = ERROR_CODE_INVALID_NAME;
                        break;
                    }
                }
                break;
         

            case CONSTANT_Utf8:

                /* We don't need these after loading time */
                secondPassConstantPoolTags[cpIndex] = 0;
                break;

            default: 
                ret = ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
                break;
        }
        if(ret != SUCCESS) {
            cleanUpFirstPool(firstPassPool, firstPassConstantPooltags, constantCount - 1);
            memoryFree(secondPassPool);
            return ret;
        }
    }
    
    *ppOrigPool = firstPassPool;
    *ppPool = secondPassPool;
    return SUCCESS;
}


static RETURN_CODE preloadSuperClass(PRELOAD_DATA pIterativeLoadData)
{
    UINT16 u2;
    CONSTANT_POOL pConstantPool = pIterativeLoadData->pConstantPool;
    FIRST_PASS_CONSTANT_POOL pFirstConstantPool = pIterativeLoadData->pFirstConstantPool;
    FILE_ACCESS pFileAccess = pIterativeLoadData->pFileAccess;
    CONSTANT_UTF8_STRING pString;
    
    /* get the super class */
    u2 = pFileAccess->pFileAccessMethods->loadU2(pFileAccess);
    if(u2 == 0 || pIterativeLoadData->isInterface) {
        pIterativeLoadData->superClassID.packageName = pIterativeLoadData->superClassID.name = NULL;
        pIterativeLoadData->superClassID.packageNameLength = pIterativeLoadData->superClassID.nameLength = 0;
    }
    else {
        if(isInvalidConstantPoolEntry(pConstantPool, u2, CONSTANT_Class)) {
            return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
        }
        pIterativeLoadData->superClassID.key = getConstantPoolNamePackage(pConstantPool, u2); /* ANSI union assignment */
        pString = pFirstConstantPool[pFirstConstantPool[u2].index].pUtf8String;        
        parseClassName(pString->utf8String, pString->length, &pIterativeLoadData->superClassID.packageName, 
            &pIterativeLoadData->superClassID.packageNameLength, &pIterativeLoadData->superClassID.name, &pIterativeLoadData->superClassID.nameLength);
    }
    return SUCCESS;
}


static RETURN_CODE preloadInterfaces(PRELOAD_DATA pIterativeLoadData)
{
    FILE_ACCESS pFileAccess = pIterativeLoadData->pFileAccess;
    CLASS_ID pNewInterfaces;
    CLASS_ID pCurrentInterface;
    UINT16 u2;
    UINT16 i;
    UINT16 newInterfaceCount;
    CONSTANT_POOL pConstantPool = pIterativeLoadData->pConstantPool;
    FIRST_PASS_CONSTANT_POOL pFirstConstantPool = pIterativeLoadData->pFirstConstantPool;
    CONSTANT_UTF8_STRING pString;
    UINT16 (*u2Loader) (FILE_ACCESS) = pFileAccess->pFileAccessMethods->loadU2;
    
    newInterfaceCount = u2Loader(pFileAccess);
    if(newInterfaceCount) {
        pNewInterfaces = pIterativeLoadData->pSuperInterfaces = (CLASS_ID) memoryAlloc(sizeof(classIdentifierStruct) * newInterfaceCount);
        if(pNewInterfaces == NULL) {
            return ERROR_CODE_OUT_OF_MEMORY;
        }
        pIterativeLoadData->superInterfaceCount = newInterfaceCount;
    }
    else {
        pNewInterfaces = pIterativeLoadData->pSuperInterfaces = NULL;
        pIterativeLoadData->superInterfaceCount = 0;
    }
    for(i=0; i<newInterfaceCount; i++) {
        pCurrentInterface = pNewInterfaces + i;
        u2 = u2Loader(pFileAccess);
        if(isInvalidConstantPoolEntry(pConstantPool, u2, CONSTANT_Class)) {
            return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
        }
        pCurrentInterface->key = getConstantPoolNamePackage(pConstantPool, u2); /* ANSI union assignment */
        pString = pFirstConstantPool[pFirstConstantPool[u2].index].pUtf8String;
        parseClassName(pString->utf8String, pString->length, &pCurrentInterface->packageName, 
            &pCurrentInterface->packageNameLength, &pCurrentInterface->name, &pCurrentInterface->nameLength);
    }
    return SUCCESS;
}


/* iterative load data will have set its class id */
RETURN_CODE preloadClass(PRELOAD_DATA pPreloadData)
{
    jumpBufferStruct errorJump;
    RETURN_CODE ret;
    UINT16 u2;
    UINT32 u4;
    FILE_ACCESS pFileAccess;
    CONSTANT_POOL pConstantPool;
    BOOLEAN isInterface;
    UINT16 (*u2Loader) (FILE_ACCESS);
    UINT32 (*u4Loader) (FILE_ACCESS);
    
    ret = openClassFile(pPreloadData->classID.packageName, pPreloadData->classID.packageNameLength, pPreloadData->classID.name, pPreloadData->classID.nameLength, &pFileAccess); 
    if(ret != SUCCESS) {
        return ret;
    }
    pPreloadData->pFileAccess = pFileAccess;
    u2Loader = pFileAccess->pFileAccessMethods->loadU2;
    u4Loader = pFileAccess->pFileAccessMethods->loadU4;
    

    /* we will jump back here if there is a file read error */
    pFileAccess->pErrorJump = &errorJump;
    if(setJump(errorJump) != 0) {
        pFileAccess->pErrorJump = NULL;
        return pFileAccess->errorFlag;
    }

    /* the magic number */
    u4 = u4Loader(pFileAccess);
    if(u4 != 0xCAFEBABE) {
        return ERROR_CODE_INVALID_MAGIC_NUMBER;
    }
    
    /* the version numbers */
    u4Loader(pFileAccess);

    /* the constant pool */
    ret = preloadConstantPool(pFileAccess, &pConstantPool, &pPreloadData->pFirstConstantPool);
    if(ret != SUCCESS) {
        return ret;
    }
    pPreloadData->pConstantPool = pConstantPool;

    /* the preloadConstantPool function resets the jump buffer, so we must reset it again here
     * for the rest of the class loading
     */
    pFileAccess->pErrorJump = &errorJump;
    
    /* access flags */
    pPreloadData->accessFlags = u2 = u2Loader(pFileAccess);
    isInterface = pPreloadData->isInterface = ((u2 & ACC_INTERFACE) != 0);
    
    if(isInterface ? ((u2 & ACC_FINAL_ABSTRACT) != ACC_ABSTRACT) : 
                      ((u2 & ACC_FINAL_ABSTRACT) == ACC_FINAL_ABSTRACT)) {
        return ERROR_CODE_INVALID_FLAGS;
    }

    /* get the constant pool index for this class */
    u2 = u2Loader(pFileAccess);
    if(isInvalidConstantPoolEntry(pConstantPool, u2, CONSTANT_Class)) {
        return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
    }
    
    if(pPreloadData->classID.key.namePackageKey != getConstantPoolNamePackage(pConstantPool, u2).namePackageKey) {
        return ERROR_CODE_INVALID_SELF_REFERENCE;
    }

    /* super class */
    ret = preloadSuperClass(pPreloadData);
    if(ret != SUCCESS) {
        return ret;
    }
    
    /* super interfaces */
    ret = preloadInterfaces(pPreloadData);
    if(ret != SUCCESS) {
        return ret;
    }

    return SUCCESS;
}


