
#include "memory/memoryAlloc.h"
#include "interpret/javaGlobals.h"
#include "string/stringKeys.h"
#include "log/logItem.h"
#include "thread/thread.h"
#include "class/table.h"
#include "class/nativeTable.h"
#include "resolution/constantPool.h"
#include "preLoad.h"
#include "iterativeLoad.h"
#include "construct.h"
#include "load.h"


/* TODO: check for errors I've missed, like a class implementing itself, etc... */

static void releaseMemory(LOAD_DATA pLoadData) 
{
    if(pLoadData->pSuperInterfaces != NULL) {
        memoryFree(pLoadData->pSuperInterfaces);
    }
    if(pLoadData->pFields != NULL) {
        memoryFree(pLoadData->pFields);
    }
    if(pLoadData->pMethods != NULL) {
        memoryFree(pLoadData->pMethods);
    }
    return;
}


static BOOLEAN lookupVirtualMethod(COMMON_CLASS_DEF pClassDef, NameTypeKey key, UINT16 *index) 
{
    UINT16 i;
    UINT16 count = pClassDef->virtualMethodCount;
    METHOD_DEF *ppMethod = getInstanceMethodList(pClassDef)->ppMethods;

    for(i=0; i<count; i++) {
        if(key == ppMethod[i]->base.nameType.nameTypeKey) {
            *index = i;
            return TRUE;
        }
    }
    return FALSE;
}

static RETURN_CODE skipAttributes(FILE_ACCESS pFileAccess, FIRST_PASS_CONSTANT_POOL pFirstConstantPool)
{
    /* skip over all attributes */
    UINT16 i;
    UINT16 u2;
    UINT16 (*u2Loader) (FILE_ACCESS) = pFileAccess->pFileAccessMethods->loadU2;
    UINT32 (*u4Loader) (FILE_ACCESS) = pFileAccess->pFileAccessMethods->loadU4;
    BOOLEAN (*skipper) (FILE_ACCESS pFileAccess, UINT32 count) = pFileAccess->pFileAccessMethods->skipBytes;
    UINT16 count = u2Loader(pFileAccess);
    
    for(i=0; i<count; i++) {
        u2 = u2Loader(pFileAccess);

        /* check the attribute name */
        if(isInvalidConstantPoolEntry(pFirstConstantPool, u2, CONSTANT_Utf8)) {
            return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
        }
        skipper(pFileAccess, u4Loader(pFileAccess));
    }
    return SUCCESS;
}

static RETURN_CODE loadMethods(FILE_ACCESS pFileAccess, CONSTANT_POOL pConstantPool, 
                       FIRST_PASS_CONSTANT_POOL pFirstConstantPool, BOOLEAN isInterface,
                       LOAD_DATA pLoadData, METHOD_DATA pMethodDataArray, UINT16 methodCount)
{
    NameType nameType;
    UINT16 accessFlags;
    UINT16 i, j, k, count, count2, count3;
    UINT16 u2;
    UINT32 u4;
    CONSTANT_UTF8_STRING pNameString, pConstantString;
    UTF8_STRING pUtf8PoolString;
    UTF8_TYPE_STRING pUtf8TypePoolString;
    RETURN_CODE ret;
    METHOD_DATA pCurrentMethodData;
    BOOLEAN foundExceptionAtt;
    UINT16 (*u2Loader) (FILE_ACCESS) = pFileAccess->pFileAccessMethods->loadU2;
    UINT32 (*u4Loader) (FILE_ACCESS) = pFileAccess->pFileAccessMethods->loadU4;
    BOOLEAN (*skipper) (FILE_ACCESS pFileAccess, UINT32 count) = pFileAccess->pFileAccessMethods->skipBytes;
    UINT32 (*bytesLoader) (FILE_ACCESS, UINT32, UINT8 *) = pFileAccess->pFileAccessMethods->loadBytes;
    
    


    for(i=0; i< methodCount; i++) {

        foundExceptionAtt = FALSE;
        pCurrentMethodData = pMethodDataArray + i;
        pCurrentMethodData->accessFlags = accessFlags = u2Loader(pFileAccess);
        
        /* only one type of access is permitted */
        if(accessFlags & ACC_PRIVATE) {
            if(accessFlags & ACC_PUBLIC_PROTECTED)
                return ERROR_CODE_INVALID_FLAGS;
        }
        else if((accessFlags & ACC_PUBLIC_PROTECTED) == ACC_PUBLIC_PROTECTED) {
            return ERROR_CODE_INVALID_FLAGS;
        }
        
        
        /* get the name */
        u2 = u2Loader(pFileAccess); 
        
        if(isInvalidConstantPoolEntry(pFirstConstantPool, u2, CONSTANT_Utf8)) {
            return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
        }
        pNameString = pFirstConstantPool[u2].pUtf8String;
        if(pNameString->length == 0) {
            return ERROR_CODE_INVALID_NAME;
        }
        if (pNameString->utf8String[0] == '<') {
            if(pNameString->length >= 6 && pNameString->utf8String[pNameString->length - 1] == '>') {
                if(!verifyMethodName(pNameString->utf8String + 1, (UINT16) (pNameString->length - 2))) {
                    ret = ERROR_CODE_INVALID_NAME;
                    break;
                }
            }
            else {
                ret = ERROR_CODE_INVALID_NAME;
                break;
            }
        }
        else if(!verifyMethodName(pNameString->utf8String, pNameString->length)) {
            return ERROR_CODE_INVALID_NAME;
        }
        if(SUCCESS != (ret = getUTF8StringByChars(pNameString->utf8String, pNameString->length, &pUtf8PoolString))) {
            return ret;
        }
        nameType.nt.nameKey = pUtf8PoolString->nameKey;

        
        /* get the type */
        u2 = u2Loader(pFileAccess); 
        if(isInvalidConstantPoolEntry(pFirstConstantPool, u2, CONSTANT_Utf8)) {
            return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
        }
        pConstantString = pFirstConstantPool[u2].pUtf8String;
        if(!verifyMethodType(pConstantString->utf8String, pConstantString->length, &pCurrentMethodData->parameterVarCount)) {
            return ERROR_CODE_INVALID_METHOD_TYPE;
        }
        if(SUCCESS != (ret = getMethodTypeString(pConstantString->utf8String, pConstantString->length, &pUtf8TypePoolString))) {
            return ret;
        }
        nameType.nt.typeKey = pUtf8TypePoolString->typeKey;
        pCurrentMethodData->nameType = nameType; /* ANSI union assignment */
        
        if (pNameString->utf8String[0] == '<') {
            if(!verifyInitializerMethodName(nameType, pConstantString->utf8String, pConstantString->length)) {
                return ERROR_CODE_INVALID_NAME;
            }
        }

        if(initNameType.nt.nameKey == nameType.nt.nameKey) {
            if(accessFlags & ACC_ABSTRACT_NATIVE_FINAL_SYNCHRONIZED_STATIC) {
                return ERROR_CODE_INVALID_FLAGS;
            }
            pCurrentMethodData->accessFlags |= ACC_INIT;
        }

        
        if(accessFlags & ACC_STATIC) {
            /* remember that an interface can have a static method, the clinit method */
            pCurrentMethodData->isStatic = TRUE;
            pLoadData->staticMethodCount++;
            if(clinitNameType.nt.nameKey == nameType.nt.nameKey) {
                pLoadData->accessFlags |= ACC_INIT;
                pLoadData->clinitMethod = pCurrentMethodData;
                pCurrentMethodData->accessFlags |= ACC_INIT;
            }
            
        }
        else {
            pCurrentMethodData->parameterVarCount++; /* the object itself also counts as a parameter */
            pLoadData->instanceMethodCount++;
            if(!(accessFlags & ACC_PRIVATE) && initNameType.nt.nameKey != nameType.nt.nameKey) {
                /* the method is not a constructor, and is neither static, private or final so it can be overridden */
                pLoadData->virtualMethodCount++;
                pCurrentMethodData->isVirtual = TRUE;
                if(accessFlags & ACC_FINAL) {
                    pLoadData->finalVirtualMethodCount++;
                }
                else if(accessFlags & ACC_ABSTRACT) {
                    pCurrentMethodData->isAbstract = TRUE;
                    pLoadData->abstractMethodCount++;
                }
                /* check if a virtual method overrides a method in the super class */
                if(pLoadData->superVirtualCount) {
                    if(lookupVirtualMethod(&pLoadData->pSuperClass->header, pCurrentMethodData->nameType.nameTypeKey, &pCurrentMethodData->superClassOverriddenIndex)) {
                        METHOD_DEF overriddenMethod = pCurrentMethodData->pOverriddenMethod = getInstanceMethod(&pLoadData->pSuperClass->header, pCurrentMethodData->superClassOverriddenIndex);
                        if(overriddenMethod->classBase.accessFlags & ACC_FINAL) {
                            return ERROR_CODE_OVERRIDDEN_FINAL_METHOD;
                        }
                        pLoadData->overridenCount++;
                    }
                }
            }
        }

        
        /* interface methods must be public and abstract */
        if(isInterface) {
            if(accessFlags != ACC_PUBLIC_ABSTRACT && pCurrentMethodData != pLoadData->clinitMethod) {
                return ERROR_CODE_INVALID_FLAGS;
            }
        }
        /* abstract methods cannot be strict, native, final, synch, private or static */
        else if((accessFlags & ACC_ABSTRACT) && (accessFlags & ACC_STRICT_NATIVE_FINAL_SYNCHRONIZED_PRIVATE_STATIC)) {
            return ERROR_CODE_INVALID_FLAGS;
        }
        else {
            if(accessFlags & ACC_NATIVE) {
                pCurrentMethodData->isNative = TRUE;
                pLoadData->nativeMethodCount++;

#if EARLY_NATIVE_LINKING         
                if((ret = getNativeFunctionIndex(pLoadData->key.namePackageKey, nameType.nameTypeKey, &pCurrentMethodData->nativeIndex)) != SUCCESS) {
                    return ret;
                }
#endif
            }
            if(accessFlags & ACC_SYNCHRONIZED) {
                pCurrentMethodData->isSynchronized = TRUE;
            }
        }

        /* get the type's parameter map, the bit-map indicating which parameters are objects */
        if(pCurrentMethodData->parameterVarCount && !pCurrentMethodData->isAbstract && !pCurrentMethodData->isNative) {
            u2 = getMethodParameterMapSize(pCurrentMethodData->parameterVarCount);
            pCurrentMethodData->pMethodParameterMap = memoryAlloc(u2);
            createMethodTypeParameterMap(pConstantString->utf8String, pConstantString->length, (BOOLEAN) !pCurrentMethodData->isStatic, (UINT16) ((u2 + 0x03) >> 2), pCurrentMethodData->pMethodParameterMap); 
            pLoadData->totalParameterMapsSize += u2;
        }

        

        /* now we need to check for the exception and code attributes */
        count = u2Loader(pFileAccess);

        

        for(j=0; j<count; j++) {

            /* get the attribute name */
            u2 = u2Loader(pFileAccess);
            if(isInvalidConstantPoolEntry(pFirstConstantPool, u2, CONSTANT_Utf8)) {
                return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
            }
            pConstantString = pFirstConstantPool[u2].pUtf8String;
            
            

                
            /* check for the code attribute */
            if(memoryCompare(pConstantString->utf8String, "Code", pConstantString->length)==0) {
                if(pCurrentMethodData->codeLength > 0 || pCurrentMethodData->isAbstract || pCurrentMethodData->isNative) {
                    return ERROR_CODE_INVALID_ATTRIBUTE;
                }
                u4Loader(pFileAccess);
                pCurrentMethodData->maxStack = u2Loader(pFileAccess);
                pCurrentMethodData->maxLocals = u2Loader(pFileAccess);
                pCurrentMethodData->codeLength = u4 = u4Loader(pFileAccess);
                if(u4 == 0) {
                    return ERROR_CODE_NO_METHOD_CODE;
                }

                

                if(NULL == (pCurrentMethodData->pCode = memoryAlloc(u4))) {
                    return ERROR_CODE_OUT_OF_MEMORY;
                }

                bytesLoader(pFileAccess, u4, pCurrentMethodData->pCode);
                pLoadData->totalCodeLength += u4;
                count3 = pCurrentMethodData->exceptionHandlerCount = u2Loader(pFileAccess);
                if(count3 > 0) {
                    pLoadData->totalExceptionHandlers += count3;
                    pLoadData->totalExceptionHandlerLists++;
                    if(NULL == (pCurrentMethodData->pExceptionHandlers = memoryAlloc(count3 * sizeof(throwableHandlerStruct)))) {
                        return ERROR_CODE_OUT_OF_MEMORY;
                    }
                    else {
                        THROWABLE_HANDLER pThrowableHandler;
                        
                        for(k=0; k<count3; k++) {
                            pThrowableHandler =  pCurrentMethodData->pExceptionHandlers + k;
                            pThrowableHandler->startPCIndex = u2Loader(pFileAccess);
                            pThrowableHandler->endPCIndex = u2Loader(pFileAccess);
                            pThrowableHandler->handlerPCIndex = u2Loader(pFileAccess);
                            u2 = u2Loader(pFileAccess);
                            /* u2 == 0 means all throwables are caught */
                            if(u2 == 0) {
                                pThrowableHandler->catchKey.namePackageKey = 0;
                                continue;
                            }
                            if(isInvalidConstantPoolEntry(pConstantPool, u2, CONSTANT_Class)) {
                                return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
                            }
                            pThrowableHandler->catchKey = getConstantPoolNamePackage(pConstantPool, u2); /* ANSI union assignment */
                        }
                    }
                }

                if((ret = skipAttributes(pFileAccess, pFirstConstantPool)) != SUCCESS) {
                    return ret;
                }

            }
            else if(memoryCompare(pConstantString->utf8String, "Exceptions", pConstantString->length)==0) {
                if(foundExceptionAtt) {
                    return ERROR_CODE_INVALID_ATTRIBUTE;
                }
                foundExceptionAtt = TRUE;
                u4Loader(pFileAccess);
                count2 = u2Loader(pFileAccess);
                for(k=0; k<count2; k++) {
                    u2 = u2Loader(pFileAccess);
                    if(isInvalidConstantPoolEntry(pFirstConstantPool, u2, CONSTANT_Class)) {
                        return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
                    }
                }
            }
            else {
                /* skip over the remaining bytes in this attribute */
                skipper(pFileAccess, u4Loader(pFileAccess));
            }
        } /* end for - attributes */
        if(pCurrentMethodData->pCode == NULL && !(pCurrentMethodData->isAbstract || pCurrentMethodData->isNative)) {
            return ERROR_CODE_NO_METHOD_CODE;
        }


    } /* end for - methods */


    /* check for duplicates */
    for(i=0; i<methodCount - 1; i++) {
        for(j=i+1; j<methodCount; j++) {
            if(pMethodDataArray[i].nameType.nameTypeKey == pMethodDataArray[j].nameType.nameTypeKey) {
                return ERROR_CODE_DUPLICATE_METHOD;
            }
        }
    }


    return SUCCESS;
}

static RETURN_CODE loadFields(FILE_ACCESS pFileAccess, CONSTANT_POOL pConstantPool, 
                       FIRST_PASS_CONSTANT_POOL pFirstConstantPool, BOOLEAN isInterface,
                       LOAD_DATA pLoadData, FIELD_DATA pFieldDataArray, UINT16 fieldCount)
{
    CONSTANT_UTF8_STRING pConstantString;
    CONSTANT_UTF8_STRING pConstantTypeString;
    UTF8_STRING pUtf8PoolString;
    UTF8_TYPE_STRING pUTF8TypeString;
    UINT16 i, j, count;
    UINT8 u1;
    UINT16 u2;
    RETURN_CODE ret;
    BOOLEAN isStatic;
    FIELD_DATA pCurrentFieldData;
    UINT16 (*u2Loader) (FILE_ACCESS) = pFileAccess->pFileAccessMethods->loadU2;
    UINT32 (*u4Loader) (FILE_ACCESS) = pFileAccess->pFileAccessMethods->loadU4;
    BOOLEAN (*skipper) (FILE_ACCESS pFileAccess, UINT32 count) = pFileAccess->pFileAccessMethods->skipBytes;
    
    
    for(i=0; i<fieldCount; i++) {

        pCurrentFieldData = pFieldDataArray + i;

        pCurrentFieldData->accessFlags = u2 = u2Loader(pFileAccess);
        
        /* only one type of access is permitted */
        if(u2 & ACC_PRIVATE) {
            if(u2 & ACC_PUBLIC_PROTECTED)
                return ERROR_CODE_INVALID_FLAGS;
        }
        else if(   (u2 & ACC_PUBLIC_PROTECTED) == ACC_PUBLIC_PROTECTED) {
            return ERROR_CODE_INVALID_FLAGS;
        }
        if((u2 & ACC_FINAL_VOLATILE) == ACC_FINAL_VOLATILE
                || (isInterface && (u2 & ACC_PUBLIC_STATIC_FINAL) != ACC_PUBLIC_STATIC_FINAL)) {
            return ERROR_CODE_INVALID_FLAGS;
        }
        pCurrentFieldData->isStatic = isStatic = ((u2 & ACC_STATIC) != 0);
        isStatic ? (pLoadData->staticFieldCount++) : (pLoadData->instanceFieldCount++);
        
        /* get the name */
        u2 = u2Loader(pFileAccess);
        
        if(isInvalidConstantPoolEntry(pFirstConstantPool, u2, CONSTANT_Utf8)) {
            return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
        }
        pConstantString = pFirstConstantPool[u2].pUtf8String;
        if(!verifyFieldName(pConstantString->utf8String, pConstantString->length)) {
            return ERROR_CODE_INVALID_NAME;
        }
        if(SUCCESS != (ret = getUTF8StringByChars(pConstantString->utf8String, pConstantString->length, &pUtf8PoolString))) {
            return ret;
        }
        pCurrentFieldData->nameType.nt.nameKey = pUtf8PoolString->nameKey;

        
        /* get the type */
        u2 = u2Loader(pFileAccess); 
        if(isInvalidConstantPoolEntry(pFirstConstantPool, u2, CONSTANT_Utf8)) {
            return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
        }
        pConstantTypeString = pFirstConstantPool[u2].pUtf8String;
        if(!verifyFieldType(pConstantTypeString->utf8String, pConstantTypeString->length)) {
            return ERROR_CODE_INVALID_FIELD_TYPE;
        }
        if(SUCCESS != (ret = getFieldTypeString(pConstantTypeString->utf8String, pConstantTypeString->length, &pUTF8TypeString))) {
            return ret;
        }
        pCurrentFieldData->nameType.nt.typeKey = pUTF8TypeString->typeKey;

        switch(pConstantTypeString->utf8String[0]) {
            case ARRAY_FIELD_TYPE:
            case OBJECT_FIELD_TYPE:
                pCurrentFieldData->isObject = TRUE;
                if(isStatic) {
                    pLoadData->objectStaticFieldCount++;
                }
                break;
#if IMPLEMENTS_FLOAT
            case DOUBLE_FIELD_TYPE:
#endif
            case LONG_FIELD_TYPE:
                pCurrentFieldData->isDoublePrimitive = TRUE;
                pCurrentFieldData->accessFlags |= ACC_DOUBLE_PRIMITIVE;
                if(isStatic) {
                    pLoadData->doubleStaticFieldCount++;
                }
                break;
            default:
                pCurrentFieldData->accessFlags |= ACC_PRIMITIVE;
        }

        /* now we need to check for the constant attributes, which can only apply to static fields */
        count = u2Loader(pFileAccess);
        for(j=0; j<count; j++) {

            /* get the attribute name */
            u2 = u2Loader(pFileAccess);
            if(isInvalidConstantPoolEntry(pFirstConstantPool, u2, CONSTANT_Utf8)) {
                return ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY;
            }
            pConstantString = pFirstConstantPool[u2].pUtf8String;
            
            /* check for the constant attribute */
            if(memoryCompare(pConstantString->utf8String, "ConstantValue", pConstantString->length)==0) {
                if(pCurrentFieldData->isConstant) {
                    /* this is the second constant value attribute */
                   return ERROR_CODE_INVALID_ATTRIBUTE;
                }
                pCurrentFieldData->accessFlags |= ACC_INIT;
                pCurrentFieldData->isConstant = TRUE;
                pLoadData->constantFieldCount++;
                if(!isStatic) {
                    return ERROR_CODE_INVALID_ATTRIBUTE;
                }
                /* the length of this attribute must be 2 */
                if(u4Loader(pFileAccess) != 2) {
                    return ERROR_CODE_INVALID_ATTRIBUTE;
                }
                /* the constant pool index for the constant value is next */
                pCurrentFieldData->constantConstantPoolIndex = u2 = u2Loader(pFileAccess);
                u1 = getConstantPoolTag(pConstantPool, u2);
                switch(pConstantTypeString->utf8String[0]) {
                    case BYTE_FIELD_TYPE:
                    case CHAR_FIELD_TYPE:
                    case INT_FIELD_TYPE:
                    case SHORT_FIELD_TYPE:
                    case BOOLEAN_FIELD_TYPE:
                        if(u1 != CONSTANT_Integer) {
                            return ERROR_CODE_INVALID_STATIC_CONSTANT;
                        }
                        break;
                    case OBJECT_FIELD_TYPE:
                        /* the only possible object constants are strings */
                        if(memoryCompare("java/lang/String;", pConstantTypeString->utf8String + 1, pConstantTypeString->length - 1) != 0
                            || u1 != CONSTANT_String) {
                            return ERROR_CODE_INVALID_STATIC_CONSTANT;
                        }

                        break;
                    case LONG_FIELD_TYPE:
                        if(u1 != CONSTANT_Long) {
                            return ERROR_CODE_INVALID_STATIC_CONSTANT;
                        }
                        break;
#if IMPLEMENTS_FLOAT
                    case FLOAT_FIELD_TYPE:
                        if(u1 != CONSTANT_Float) {
                            return ERROR_CODE_INVALID_STATIC_CONSTANT;
                        }
                        break;
                    case DOUBLE_FIELD_TYPE:
                        if(u1 != CONSTANT_Double) {
                            return ERROR_CODE_INVALID_STATIC_CONSTANT;
                        }
                        break;
#endif
                    case ARRAY_FIELD_TYPE:
                        /* the only possible object constants are strings */
                        return ERROR_CODE_INVALID_STATIC_CONSTANT;
                    default:
                        return ERROR_CODE_INVALID_FIELD_TYPE;
                }
                
            }
            else {
                /* skip over the remaining bytes in this attribute */
                skipper(pFileAccess, u4Loader(pFileAccess));
            }

        } /* end for */

    } /* end for */
    
    /* check for duplicates - according to the spec, no two fields may have the same
     * name and type - although it is not apparent to me how two fields may actually have the 
     * same name within the same class
     */
    for(i=0; i<fieldCount - 1; i++) {
        for(j=i+1; j<fieldCount; j++) {
            if(pFieldDataArray[i].nameType.nameTypeKey == pFieldDataArray[j].nameType.nameTypeKey) {
                return ERROR_CODE_DUPLICATE_FIELD;
            }
        }
    }

    
    return SUCCESS;

}

static BOOLEAN interfaceNotInList(LOADED_INTERFACE_DEF *pList, UINT16 listSize, LOADED_INTERFACE_DEF pInterface) 
{
    UINT16 i;
    for(i=0; i<listSize; i++) {
        if(pList[i] == pInterface) {
            return FALSE;
        }
    }
    return TRUE;
}


static RETURN_CODE loadInterfaces(PRELOAD_DATA pPreloadData, LOAD_DATA pLoadData)
{
    UINT16 i, count;
    UINT16 totalInterfaces;
    UINT16 newInterfaceCount;
    INTERFACE_LIST pInterfaceList;
    LOADED_INTERFACE_DEF *pNewInterfaces;
    COMMON_CLASS_DEF pTempClass;
    CLASS_ID pCurrentInterface;
    BOOLEAN isInterface = pLoadData->isInterface;

    newInterfaceCount = pPreloadData->superInterfaceCount;
    if(newInterfaceCount) {
        pNewInterfaces = memoryAlloc(sizeof(LOADED_INTERFACE_DEF) * newInterfaceCount);
        if(pNewInterfaces == NULL) {
            return ERROR_CODE_OUT_OF_MEMORY;
        }
    }
    else {
        pNewInterfaces = NULL;
    }

    totalInterfaces = newInterfaceCount;
    
    if(newInterfaceCount) {
        for(i=0; i<newInterfaceCount; i++) {
            pCurrentInterface = pPreloadData->pSuperInterfaces + i;
            pTempClass = getClassDefInTable(pCurrentInterface->key.namePackageKey);
            if(isNotInterface(pTempClass)) {
                return ERROR_CODE_INVALID_SUPER_INTERFACE;
            }
            pNewInterfaces[i] = (LOADED_INTERFACE_DEF) pTempClass;
        }
        for(i=0; i<newInterfaceCount; i++) {
            pInterfaceList = pNewInterfaces[i]->loadedHeader.pSuperInterfaceList;
            if(pInterfaceList) {
                totalInterfaces += pInterfaceList->length;
            }
        }
    }

    /* 
     * for the interface list, need to account for interfaces implemented by the super class 
     * as well as interfaces extended by interfaces listed in the class file 
     */
    
    if(!isInterface && pLoadData->pSuperClass != NULL) {
        pInterfaceList = pLoadData->pSuperClass->loadedHeader.pSuperInterfaceList;
        if(pInterfaceList) {
            totalInterfaces += pInterfaceList->length;
        }
    }
    
    
    if(totalInterfaces > newInterfaceCount) {

        LOADED_INTERFACE_DEF *pAllInterfaces;

        pAllInterfaces = memoryAlloc(sizeof(LOADED_INTERFACE_DEF) * totalInterfaces);
        if(pAllInterfaces == NULL) {
            return ERROR_CODE_OUT_OF_MEMORY;
        }

        count = newInterfaceCount;
        if(newInterfaceCount) {
            memoryCopy(pAllInterfaces, pNewInterfaces, newInterfaceCount * sizeof(LOADED_INTERFACE_DEF));
        }
        if(!isInterface && pLoadData->pSuperClass != NULL) {
            pInterfaceList = pLoadData->pSuperClass->loadedHeader.pSuperInterfaceList;
            if(pInterfaceList) {
                for(i=0; i<pInterfaceList->length; i++) {
                    if(interfaceNotInList(pAllInterfaces, count, pInterfaceList->pInterfaceDefs[i])) {
                        pAllInterfaces[count++] = pInterfaceList->pInterfaceDefs[i];
                    }
                }
            }
        }
        for(i=0; i<newInterfaceCount; i++) {
            pInterfaceList = pNewInterfaces[i]->loadedHeader.pSuperInterfaceList;
            if(pInterfaceList) {
                for(i=0; i<pInterfaceList->length; i++) {
                    if(interfaceNotInList(pAllInterfaces, count, pInterfaceList->pInterfaceDefs[i])) {
                        pAllInterfaces[count++] = pInterfaceList->pInterfaceDefs[i];
                    }
                }
            }
        }
        if(count == newInterfaceCount) {
            /* we haven't found any new interfaces, they were all duplicates of old ones */
            pLoadData->superInterfaceCount = newInterfaceCount;
            pLoadData->pSuperInterfaces = pNewInterfaces;
            memoryFree(pAllInterfaces);
        }
        else {
            pLoadData->superInterfaceCount = count;
            pLoadData->pSuperInterfaces = pAllInterfaces;
            if(newInterfaceCount) {
                memoryFree(pNewInterfaces);
            }
        }
    }
    else {
        pLoadData->superInterfaceCount = newInterfaceCount;
        pLoadData->pSuperInterfaces = pNewInterfaces;
    }

    return SUCCESS;
}

static RETURN_CODE loadSuperClass(PRELOAD_DATA pPreloadData, LOAD_DATA pLoadData)
{
    BOOLEAN isInterface = pLoadData->isInterface;

    /* get the super class */
    if(pPreloadData->superClassID.nameLength == 0) {
        /* we are loading java.lang.Object, which has no super class */
        if(!isInterface && pLoadData->key.namePackageKey != javaLangObjectKey.namePackageKey) {
            return ERROR_CODE_INVALID_SUPER_CLASS;
        }
        pLoadData->pSuperClass = NULL;
        pLoadData->superVirtualCount = 0;
    }
    else {
        COMMON_CLASS_DEF pTempClass = getClassDefInTable(pPreloadData->superClassID.key.namePackageKey);

        if((isInterface && pTempClass != pJavaLangObject->pRepresentativeClassDef) || isArrayOrInterface(pTempClass)
            || !isSuperClassOrInterfaceAccessible(pLoadData->key.np.packageKey, pTempClass)) {
            return ERROR_CODE_INVALID_SUPER_CLASS;
        }
        if(!isInterface) {
            pLoadData->pSuperClass = (LOADED_CLASS_DEF) pTempClass;
            pLoadData->superVirtualCount = pLoadData->pSuperClass->header.virtualMethodCount;
        }
        else {
            pLoadData->pSuperClass = NULL;
            pLoadData->superVirtualCount = 0;
        }   
    }
    return SUCCESS;
}


static RETURN_CODE readInClassData(PRELOAD_DATA pPreloadData, LOAD_DATA pLoadData)
{
    UINT16 count;
    UINT16 accessFlags;
    BOOLEAN isInterface;
    FIELD_DATA pFieldData;
    METHOD_DATA pMethodData;
    RETURN_CODE ret;
    jumpBufferStruct errorJump;
    FILE_ACCESS pFileAccess = pPreloadData->pFileAccess;
    CONSTANT_POOL pConstantPool = pPreloadData->pConstantPool;
    FIRST_PASS_CONSTANT_POOL pFirstConstantPool = pPreloadData->pFirstConstantPool;
    UINT16 (*u2Loader) (FILE_ACCESS) = pFileAccess->pFileAccessMethods->loadU2;
    
     
    /* First accumulate data from the already loaded data, the super class and super interfaces */
    accessFlags = pLoadData->accessFlags = pPreloadData->accessFlags;
    isInterface = pLoadData->isInterface = pPreloadData->isInterface;
    pLoadData->isAbstract = ((accessFlags & ACC_INTERFACE) != 0);
#if PREEMPTIVE_THREADS
    pLoadData->pMonitor = pPreloadData->pMonitor;
#endif
    
    ret = loadSuperClass(pPreloadData, pLoadData);
    if(ret != SUCCESS) {
        return ret;
    }

    ret = loadInterfaces(pPreloadData, pLoadData);
    if(ret != SUCCESS) {
        return ret;
    }

    /* Now read the rest of the class file. We will jump back here if there is a file read error */
    pFileAccess->pErrorJump = &errorJump;
    if(setJump(errorJump) != 0) {
        pFileAccess->pErrorJump = NULL;
        return pFileAccess->errorFlag;
    }

    /* fields */
    pLoadData->newFieldCount = count = u2Loader(pFileAccess);
    if(count > 0) {
        pLoadData->pFields = pFieldData = memoryCalloc(sizeof(fieldDataStruct) * count, 1);
        if(pFieldData == NULL) {
            return ERROR_CODE_OUT_OF_MEMORY;
        }
        ret = loadFields(pFileAccess, pConstantPool, pFirstConstantPool, isInterface, pLoadData, pFieldData, count);
        if(ret != SUCCESS) {
            return ret;
        }
    }

    

    /* methods */
    pLoadData->newMethodCount = count = u2Loader(pFileAccess);
    if(count > 0) {
        pLoadData->pMethods = pMethodData = memoryCalloc(sizeof(methodDataStruct) * count, 1);
        if(pMethodData == NULL) {
            return ERROR_CODE_OUT_OF_MEMORY;
        }
        ret = loadMethods(pFileAccess, pConstantPool, pFirstConstantPool, isInterface, pLoadData, pMethodData, count);
        if(ret != SUCCESS) {
            return ret;
        }
    }

    /* attributes */
    if((ret = skipAttributes(pFileAccess, pFirstConstantPool)) != SUCCESS) {
        return ret;
    }
    
    pFileAccess->pErrorJump = NULL;
    
    if(!(pFileAccess->pFileAccessMethods->eof(pFileAccess))) {
        return ERROR_CODE_INVALID_FILE_SIZE;
    }
    return SUCCESS;
}





/* All super classes and interfaces must have been loaded without error before this function call. 
 * It is also assumed that the fileAccess object is currently pointing to the access flags in the class file.
 */
RETURN_CODE completeClassLoad(PRELOAD_DATA pPreloadData)
{
    loadDataStruct loadData; /* this is throwing a lot of stuff on the stack, move it to the heap if necessary */
    RETURN_CODE ret;
    CONSTANT_POOL pConstantPool = pPreloadData->pConstantPool;
    FIRST_PASS_CONSTANT_POOL pFirstConstantPool = pPreloadData->pFirstConstantPool;
    FILE_ACCESS pFileAccess = pPreloadData->pFileAccess;

    memorySet(&loadData, 0, sizeof(loadDataStruct));
    loadData.key = pPreloadData->classID.key;  /* ANSI union assignment */
    
    if((ret = readInClassData(pPreloadData, &loadData)) == SUCCESS) {
        if((ret = constructClass(&loadData, pConstantPool, &pPreloadData->pClassInstance, &pPreloadData->pClassDef)) == SUCCESS) {
            pPreloadData->pClassDef->header.tableIndex = getClassTableIndex(getClassTable(), pPreloadData->pClassEntry);
        }
        
    }
    releaseMemory(&loadData);
    return ret;
}


