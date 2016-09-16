
#include "javaThread/javaThread.h"
#include "memory/memoryAlloc.h"
#include "log/logItem.h"
#include "resolution/constantPool.h"
#include "prepare.h"
#include "preLoad.h"
#include "construct.h"


/* isMethodBigger(a, b) is TRUE if a > b */
#define isMethodBigger(isStatic, pMethodA, pMethodB)                                    \
    (isStatic ? (isInitializerMethod(pMethodA) ||                                       \
        ((pMethodA->base.nameType.nameTypeKey > pMethodB->base.nameType.nameTypeKey)    \
         && !isInitializerMethod(pMethodB))) :                                          \
        ((isInitializerMethod(pMethodA) && !isInitializerMethod(pMethodB)) ||           \
         (isPrivateMethod(pMethodA) && !isInitializerOrPrivateMethod(pMethodB))))

#define isFieldBigger(pFieldA, pFieldB) (isPrimitiveField(pFieldA) && isPrimitiveField(pFieldB))

/* 
 * this sort is rather simple because:
 * with non-static methods we are really only sorting amongst three categories: virtual, private, and init
 * We are sorting statics by key but there is usually a small number of statics per class
 * (since statics are not inherited there is no growth)
 */
static void sortMethods(METHOD_LIST pList, BOOLEAN isStatic) 
{
    METHOD_DEF *ppMethodArray = pList->ppMethods;
    METHOD_DEF pTempMethod;
    UINT16 highIndex = pList->length - 1;
    UINT16 i, j;
    
    for(i=highIndex; i>=1; i--) {
        for(j=i-1; j<highIndex && isMethodBigger(isStatic, ppMethodArray[j], ppMethodArray[j+1]); j++) {
            pTempMethod = ppMethodArray[j];
            ppMethodArray[j] = ppMethodArray[j+1];
            ppMethodArray[j+1] = pTempMethod;
        }
    }
    return;

}

static void sortFields(FIELD_LIST pList)
{
    FIELD_DEF *ppFieldArray = pList->ppFields;
    FIELD_DEF pTempField;
    UINT16 highIndex = pList->length - 1;
    UINT16 i, j;
    
    for(i=highIndex; i>=1; i--) {
        for(j=i-1; j<highIndex && isFieldBigger(ppFieldArray[j], ppFieldArray[j+1]); j++) {
            pTempField = ppFieldArray[j];
            ppFieldArray[j] = ppFieldArray[j+1];
            ppFieldArray[j+1] = pTempField;
        }
    }
    return;
}


static PBYTE constructMethods(LOAD_DATA pLoadData, LOADED_CLASS_DEF pClassDef, UINT16 totalInstanceMethodCount, BOOLEAN isInterface, PBYTE pNext)
{
    METHOD_LIST pInstanceList;
    METHOD_LIST pStaticList;
    THROWABLE_HANDLER *ppHandlerInClass;
    THROWABLE_HANDLER pHandlerInDataStruct;
    UINT16 i;
    UINT16 listLength;

    UINT16 currentInstanceIndex;
    UINT16 currentStaticIndex = 0;
    UINT16 currentLoadedMethodIndex = 0;
    METHOD_DATA pCurrentLoadedMethod;
    METHOD_DEF pCurrentMethodDef;

    if(totalInstanceMethodCount) {
        pInstanceList = pClassDef->header.pInstanceMethods = (METHOD_LIST) pNext;
        pNext += getMethodListSize(totalInstanceMethodCount);
        pInstanceList->length = totalInstanceMethodCount;
    }
    else {
        pInstanceList = pClassDef->header.pInstanceMethods = NULL;
    }

    pClassDef->header.virtualMethodCount = pLoadData->virtualMethodCount + 
        (isInterface ? 0 : (pLoadData->superVirtualCount - pLoadData->overridenCount));

    if(pLoadData->staticMethodCount) {
        pStaticList = pClassDef->loadedHeader.pStaticMethods = (METHOD_LIST) pNext;
        pNext += getMethodListSize(pLoadData->staticMethodCount);
        pStaticList->length = pLoadData->staticMethodCount;
    }
    else {
        pStaticList = pClassDef->loadedHeader.pStaticMethods = NULL;
    }

    if(pLoadData->superVirtualCount) {
        memoryCopy(pInstanceList->ppMethods, 
            pLoadData->pSuperClass->header.pInstanceMethods->ppMethods, 
            pLoadData->superVirtualCount * sizeof(METHOD_DEF));
        currentInstanceIndex = pLoadData->superVirtualCount;
    }
    else {
        currentInstanceIndex = 0;
    }

    while(currentLoadedMethodIndex < pLoadData->newMethodCount) {
        pCurrentLoadedMethod = pLoadData->pMethods + currentLoadedMethodIndex;
        currentLoadedMethodIndex++;
        pCurrentMethodDef = (METHOD_DEF) pNext;
        if(pCurrentLoadedMethod->isStatic) {
            pStaticList->ppMethods[currentStaticIndex++] = pCurrentMethodDef;
        }
        else {
            if(pCurrentLoadedMethod->pOverriddenMethod) {
                pInstanceList->ppMethods[pCurrentLoadedMethod->superClassOverriddenIndex] = pCurrentMethodDef;
            }
            else {
                pInstanceList->ppMethods[currentInstanceIndex++] = pCurrentMethodDef;
            }
        }
        pCurrentMethodDef->base.nameType = pCurrentLoadedMethod->nameType; /* ANSI union assignment */
        pCurrentMethodDef->base.parameterVarCount = pCurrentLoadedMethod->parameterVarCount;
        pCurrentMethodDef->base.pOwningClass = pClassDef;
        if(isInterface && pCurrentLoadedMethod != pLoadData->clinitMethod) {
            pNext += getInterfaceMethodSize();
            continue;
        }
        pCurrentMethodDef->classBase.accessFlags = pCurrentLoadedMethod->accessFlags;
        if(pCurrentLoadedMethod->isAbstract) {
            pNext += getAbstractMethodSize();
            continue;
        }
        if(pCurrentLoadedMethod->isNative) {
            NATIVE_METHOD_DEF pNativeMethod = (NATIVE_METHOD_DEF) pCurrentMethodDef;

            pNext += getNativeMethodSize();
#if EARLY_NATIVE_LINKING
            pNativeMethod->nativeFunctionIndex = pCurrentLoadedMethod->nativeIndex;
            setNativeMethodLinked(pNativeMethod);
#endif
            continue;
        }
        pNext += getClassMethodSize();       
        pCurrentMethodDef->varCount = pCurrentLoadedMethod->maxLocals;
        pCurrentMethodDef->localVarCount = pCurrentLoadedMethod->maxLocals - pCurrentLoadedMethod->parameterVarCount;
        pCurrentMethodDef->invokeCount = getInvokeSize(pCurrentLoadedMethod->maxLocals, pCurrentLoadedMethod->maxStack, pCurrentLoadedMethod->isSynchronized);
        pCurrentMethodDef->maxStackWithMonitor = pCurrentLoadedMethod->maxStack + (pCurrentLoadedMethod->isSynchronized ? 1 : 0);
        pCurrentMethodDef->byteCodeCount = pCurrentLoadedMethod->codeLength;

#if REALLOCATE_METHOD_PARAMETER_MAPS
        if(pCurrentLoadedMethod->parameterVarCount) {
            pCurrentMethodDef->pParameterMap = (UINT32 *) pNext;
            listLength = getMethodParameterMapSize(pCurrentLoadedMethod->parameterVarCount);
            memoryCopy(pNext, pCurrentLoadedMethod->pMethodParameterMap, listLength);
            pNext += listLength;
            memoryFree(pCurrentLoadedMethod->pMethodParameterMap);
        }
        else {
            pCurrentMethodDef->pParameterMap = NULL;
        }
#else
        pCurrentMethodDef->pParameterMap = pCurrentLoadedMethod->pMethodParameterMap;
#endif

#if REALLOCATE_CODE
        pCurrentMethodDef->pCode = pNext;
        memoryCopy(pNext, pCurrentLoadedMethod->pCode, pCurrentLoadedMethod->codeLength);
        memoryFree(pCurrentLoadedMethod->pCode);
        pNext += pCurrentLoadedMethod->codeLength;
#else
        pCurrentMethodDef->pCode = pCurrentLoadedMethod->pCode;
#endif

        if(pCurrentLoadedMethod->exceptionHandlerCount > 0) {
            pCurrentMethodDef->throwableHandlers = (THROWABLE_HANDLER_LIST) pNext;
            pNext += getHandlerListSize(pCurrentLoadedMethod->exceptionHandlerCount);
            pCurrentMethodDef->throwableHandlers->length = pCurrentLoadedMethod->exceptionHandlerCount;

            for(i=0; i<pCurrentLoadedMethod->exceptionHandlerCount; i++) {
                ppHandlerInClass = pCurrentMethodDef->throwableHandlers->ppHandlers + i;
                pHandlerInDataStruct = pCurrentLoadedMethod->pExceptionHandlers + i;

#if REALLOCATE_HANDLERS         
                *ppHandlerInClass = (THROWABLE_HANDLER) pNext;
                **ppHandlerInClass = *pHandlerInDataStruct; /* ANSI structure assignment */
                pNext += sizeof(throwableHandlerStruct);
#else
                *ppHandlerInClass = pHandlerInDataStruct;
#endif
            }
#if REALLOCATE_HANDLERS
            memoryFree(pCurrentLoadedMethod->pExceptionHandlers);
#endif

        }
        else {
            pCurrentMethodDef->throwableHandlers = NULL;
        }
    } /* end for - iteration through loaded methods */

    

    if(pStaticList) {
        listLength = pStaticList->length;
        if(listLength > 1) {
            sortMethods(pStaticList, TRUE);
        }
        for(i=0; i<listLength; i++) {
            getMethod(pStaticList, i)->base.methodIndex = i;
        }
        if(pLoadData->clinitMethod != NULL) {
            pStaticList->length--;
            listLength--;
        }
        
    }
    if(pInstanceList) {
        listLength = pInstanceList->length;
        if(listLength > 1) {
            sortMethods(pInstanceList, FALSE);
        }
        for(i=0; i<listLength; i++) {
            getMethod(pInstanceList, i)->base.methodIndex = i;
        }
    }

    return pNext;
}
 


static PBYTE constructFields(LOAD_DATA pLoadData, CONSTANT_POOL pConstantPool, LOADED_CLASS_DEF pClassDef, BOOLEAN isInterface, PBYTE pNext)
{
    FIELD_LIST pInstanceList;
    FIELD_LIST pStaticList;
    UINT16 currentInstanceIndex = 0; /* the non-static field index in the class */
    UINT16 currentStaticIndex; /* the static field index in the class */
    UINT16 currentLoadedFieldIndex = 0; /* the field index */
    UINT16 currentStaticPrimitiveFieldDataIndex; /* keeps track of static field data locations */
    UINT16 currentStaticObjectFieldDataIndex; /* keeps track of static field data locations */
    UINT16 currentInstancePrimitiveFieldDataIndex; /* keeps track of instance field data locations */
    UINT16 currentInstanceObjectFieldDataIndex; /* keeps track of instance field data locations */
    FIELD_DATA pCurrentLoadedField;
    FIELD_DEF pCurrentFieldDef;
    CONSTANT_FIELD_DEF pConstantFieldDef;
    CONSTANT_POOL_ENTRY pConstantPoolEntry;
            
    if(pLoadData->staticFieldCount) {
        pStaticList = pClassDef->loadedHeader.pStaticFields = (FIELD_LIST) pNext;
        pNext += getFieldListSize(pLoadData->staticFieldCount);
        pStaticList->length = pLoadData->staticFieldCount;
        pClassDef->loadedHeader.objectStaticFieldCount = pLoadData->objectStaticFieldCount;
        currentStaticPrimitiveFieldDataIndex = getStaticPrimitiveIndexOffset(pLoadData->objectStaticFieldCount);
        currentStaticObjectFieldDataIndex = 0;
        currentStaticIndex = 0;
    }
    else {
        pClassDef->loadedHeader.pStaticFields = NULL;
        pClassDef->loadedHeader.objectStaticFieldCount = 0;
        pStaticList = NULL;
    }
    
    if(isInterface) {
        pInstanceList = NULL;
    }
    else {
        LOADED_CLASS_DEF pSuperClass;
        
        if(pLoadData->instanceFieldCount) {
            pInstanceList = pClassDef->pInstanceFields = (FIELD_LIST) pNext;
            pNext += getFieldListSize(pLoadData->instanceFieldCount);
            pInstanceList->length = pLoadData->instanceFieldCount;
        }
        else {
            pInstanceList = pClassDef->pInstanceFields = NULL;
        }
        pSuperClass = pLoadData->pSuperClass;
        while(pSuperClass && !pSuperClass->pInstanceFields) {
            pSuperClass = pSuperClass->header.pSuperClass;
        }

        if(pSuperClass) {
            currentInstanceObjectFieldDataIndex = getObjectInstanceFieldCount(pSuperClass);
            /* get the instance size, figure out how much of that is the object fields and the header, the rest is be the primitive fields */
            currentInstancePrimitiveFieldDataIndex = sizeToPrimitiveFieldSize(getInstantiateSize(pSuperClass) - (currentInstanceObjectFieldDataIndex * getObjectFieldSize()));
        }
        else {
            currentInstanceObjectFieldDataIndex = currentInstancePrimitiveFieldDataIndex = 0;
        }
        
    }
  
    while(currentLoadedFieldIndex < pLoadData->newFieldCount) {
        pCurrentLoadedField = pLoadData->pFields + currentLoadedFieldIndex;
        currentLoadedFieldIndex++;
        pCurrentFieldDef = (FIELD_DEF) pNext;
        
        if(pCurrentLoadedField->isStatic) {
            pStaticList->ppFields[currentStaticIndex++] = pCurrentFieldDef;

            if(pCurrentLoadedField->isObject) {
                pCurrentFieldDef->fieldIndex = currentStaticObjectFieldDataIndex;
                currentStaticObjectFieldDataIndex++;
            }
            else {
                pCurrentFieldDef->fieldIndex = currentStaticPrimitiveFieldDataIndex;
                if(pCurrentLoadedField->isDoublePrimitive) {
                    currentStaticPrimitiveFieldDataIndex += 2;
                }
                else {
                    currentStaticPrimitiveFieldDataIndex++;
                }
            }

            if(pCurrentLoadedField->isConstant) {
                pConstantFieldDef = (CONSTANT_FIELD_DEF) pCurrentFieldDef;
                pConstantPoolEntry = getConstantPoolEntry(pConstantPool, pCurrentLoadedField->constantConstantPoolIndex);
                pConstantFieldDef->value = pConstantPoolEntry->constantValue; /* ANSI union assignment */
                if(pCurrentLoadedField->isDoublePrimitive) {
                    pConstantFieldDef->value2 = (pConstantPoolEntry + 1)->constantValue; /* ANSI union assignment */
                }
                pNext += getConstantFieldDefSize();
            }
            else {
                pNext += getFieldDefSize();   
            }
        }
        else {
            pInstanceList->ppFields[currentInstanceIndex++] = pCurrentFieldDef;
            if(pCurrentLoadedField->isObject) {
                pCurrentFieldDef->fieldIndex = currentInstanceObjectFieldDataIndex;
                currentInstanceObjectFieldDataIndex++;
            }
            else {
                pCurrentFieldDef->fieldIndex = currentInstancePrimitiveFieldDataIndex;
                if(pCurrentLoadedField->isDoublePrimitive) {
                    currentInstancePrimitiveFieldDataIndex += 2;
                }
                else {
                    currentInstancePrimitiveFieldDataIndex++;
                }
            }
            pNext += getFieldDefSize();
        }

        pCurrentFieldDef->accessFlags = pCurrentLoadedField->accessFlags;
        pCurrentFieldDef->key = pCurrentLoadedField->nameType; /* ANSI union assignment */
        pCurrentFieldDef->pOwningClass = pClassDef;
    }

    if(!isInterface) {
        pClassDef->instanceSize = getObjectInstanceSize(currentInstancePrimitiveFieldDataIndex, currentInstanceObjectFieldDataIndex);
        pClassDef->header.objectInstanceFieldCount = currentInstanceObjectFieldDataIndex;
    }
    else {
        pClassDef->header.objectInstanceFieldCount = 0;
    }


    /* this sorting is actually not necessary, but what the heck - later it could be used to make lookup quicker */
    if(pStaticList && pStaticList->length > 1) {
        sortFields(pStaticList);
    }

    if(pInstanceList && pInstanceList->length > 1) {
        sortFields(pInstanceList);
    }
    return pNext;
}


static PBYTE constructInterfaceList(LOAD_DATA pLoadData, LOADED_CLASS_DEF pClassDef, BOOLEAN isInterface, PBYTE pNext)
{
    INTERFACE_LIST pInterfaceList;
    UINT16 i;

    pInterfaceList = pClassDef->loadedHeader.pSuperInterfaceList = (INTERFACE_LIST) pNext;
    pNext += getInterfaceListSize(pLoadData->superInterfaceCount);
    pInterfaceList->length = pLoadData->superInterfaceCount;
    for(i=0; i<pLoadData->superInterfaceCount; i++) {
        pInterfaceList->pInterfaceDefs[i] = pLoadData->pSuperInterfaces[i];
    }
    if(isInterface || pLoadData->isAbstract) {
        pInterfaceList->pppInterfaceImplementationTable = NULL;
    }
    else {
        UINT16 j, k;
        METHOD_DEF pClassMethod;
        METHOD_DEF pInterfaceMethod;
        LOADED_INTERFACE_DEF pInterface;
        BOOLEAN found;

        pInterfaceList->pppInterfaceImplementationTable = (METHOD_DEF **) pNext;
        pNext += pInterfaceList->length * sizeof(METHOD_DEF *);

        /* for each implemented interface */
        for(i=0; i<pInterfaceList->length; i++) {
            pInterfaceList->pppInterfaceImplementationTable[i] = (METHOD_DEF *) pNext;
            pNext += pInterfaceList->pInterfaceDefs[i]->header.virtualMethodCount * sizeof(METHOD_DEF);
            pInterface = pInterfaceList->pInterfaceDefs[i];

            /* for each method in the implemented interface */
            for(j=0; j<pInterface->header.virtualMethodCount; j++) {
                pInterfaceMethod = getInstanceMethod(pInterface, j);
                found = FALSE;
                  
                /* and each method in the implementing class */
                for(k=0; k<pClassDef->header.virtualMethodCount; k++) {
                    pClassMethod = getInstanceMethod(pClassDef, k);
                    
                    /* check for a match */
                    if(getMethodKey(pClassMethod) == getMethodKey(pInterfaceMethod)) {
                        /* we've found a match for the table entry */
                        pInterfaceList->pppInterfaceImplementationTable[i][j] = pClassMethod;
                        found = TRUE;
                        break;
                    }
                }
                if(!found) {
                    pInterfaceList->pppInterfaceImplementationTable[i][j] = NULL;
                }
            } /* end for - interface loop */
        } /* end for - all interfaces loop */
    }
    return pNext;
}


static UINT32 calculateClassSize(LOAD_DATA pLoadData, CONSTANT_POOL pConstantPool) 
{
    UINT32 blockSize;
    UINT16 i;
    BOOLEAN isInterface = pLoadData->isInterface;
    
    
    /* account for the class def and all new method defs */
    
    if(isInterface) {
        blockSize = getLoadedInterfaceStructSize();
        if(pLoadData->clinitMethod) {
            blockSize += getClassMethodSize();
        }
        blockSize += pLoadData->abstractMethodCount * getInterfaceMethodSize();
        if(pLoadData->instanceMethodCount)
            blockSize += getMethodListSize(pLoadData->instanceMethodCount);
    }
    else {
        blockSize = getLoadedClassStructSize();
        blockSize += pLoadData->abstractMethodCount * getAbstractMethodSize();
        blockSize += pLoadData->nativeMethodCount * getNativeMethodSize();
        blockSize += (pLoadData->newMethodCount - pLoadData->abstractMethodCount - pLoadData->nativeMethodCount) * getClassMethodSize();
        
        /*
        if(pLoadData->totalInstanceFieldCount) {
            blockSize += getFieldListSize(pLoadData->totalInstanceFieldCount);
            blockSize += pLoadData->instanceFieldCount * getFieldDefSize();
        }
        */
        if(pLoadData->instanceFieldCount) {
            blockSize += getFieldListSize(pLoadData->instanceFieldCount);
            blockSize += pLoadData->instanceFieldCount * getFieldDefSize();
        }
        
        /* all non-static methods in the new class, plus all virtuals non-overriden in the super class */
        blockSize += getMethodListSize(pLoadData->instanceMethodCount + pLoadData->superVirtualCount - pLoadData->overridenCount);
        
        /* account for the static method list and field list */
        if(pLoadData->staticMethodCount) {
            blockSize += getMethodListSize(pLoadData->staticMethodCount);
        }

#if REALLOCATE_METHOD_PARAMETER_MAPS
        blockSize += pLoadData->totalParameterMapsSize;
#endif
        if(pLoadData->totalExceptionHandlers) {
            /* 
             * pretend we have (pLoadData->totalExceptionHandlerLists - 1) lists of size 1 
             * and 1 list of size (pLoadData->totalExceptionHandlers - (pLoadData->totalExceptionHandlerLists - 1))
             */
            blockSize += getHandlerListSize(1) * (pLoadData->totalExceptionHandlerLists - 1);
            blockSize += getHandlerListSize(pLoadData->totalExceptionHandlers - pLoadData->totalExceptionHandlerLists + 1);
#if REALLOCATE_HANDLERS
            blockSize += pLoadData->totalExceptionHandlers * sizeof(throwableHandlerStruct);
#endif
        }

#if REALLOCATE_CODE
        if(pLoadData->totalCodeLength) {
            blockSize += pLoadData->totalCodeLength;
        }
#endif
        
    }

    if(pLoadData->staticFieldCount) {
        blockSize += getFieldListSize(pLoadData->staticFieldCount);
        blockSize += (pLoadData->staticFieldCount - pLoadData->constantFieldCount) * getFieldDefSize();
        blockSize += pLoadData->constantFieldCount * getConstantFieldDefSize();
    }
    
    /* interface list and interface implementation table */
    if(pLoadData->superInterfaceCount) {
        blockSize += getInterfaceListSize(pLoadData->superInterfaceCount);
        
        if(!isInterface) {
            if(!pLoadData->isAbstract) {
                /* for each interface, need an array of size numMetods * sizeof(METHOD_DEF) */
                for(i=0; i<pLoadData->superInterfaceCount; i++) {
                    blockSize += pLoadData->pSuperInterfaces[i]->header.virtualMethodCount * sizeof(METHOD_DEF);
                }

                /* need an array of size numInterfaces * sizeof(METHOD_DEF *) each element will point to one of the above arrays */
                blockSize += pLoadData->superInterfaceCount * sizeof(METHOD_DEF *);
            }
        }
    }
    

#if REALLOCATE_CONSTANT_POOL
    blockSize += getConstantPoolSize(getConstantPoolLength(pConstantPool));
#endif

#if PREPARE_WHILE_LOADING
    blockSize += getClassInstanceSize((pLoadData->staticFieldCount - pLoadData->objectStaticFieldCount) + pLoadData->doubleStaticFieldCount, pLoadData->objectStaticFieldCount);
#endif

    return blockSize;
}




RETURN_CODE constructClass(LOAD_DATA pLoadData, CONSTANT_POOL pConstantPool, 
                                  CLASS_INSTANCE *ppClassInstance, LOADED_CLASS_DEF *ppClassDef)
{
#if PREPARE_WHILE_LOADING
    CLASS_INSTANCE pClassInstance;
#endif
    LOADED_CLASS_DEF pClassDef;
    UINT32 size = calculateClassSize(pLoadData, pConstantPool);
    BOOLEAN isInterface = pLoadData->isInterface;
    UINT16 totalInstanceMethodCount = pLoadData->instanceMethodCount + (isInterface ? 0 : (pLoadData->superVirtualCount - pLoadData->overridenCount));
    UINT16 methodCount = pLoadData->staticMethodCount + totalInstanceMethodCount;
    PBYTE pNext;
    PBYTE pFirst;
    
    pFirst = pNext = memoryAlloc(size);
    if(pNext == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }

/* the class instance must appear at the top of the memory block for the sake of the garbage collector.
 * That way java.lang.Class objects will look like any other object, with the header just above the object
 * fields, but in the class of the java.lang.Class object those fields will be static fields.
 */
#if PREPARE_WHILE_LOADING 
    pClassInstance = (CLASS_INSTANCE) pNext;
    pNext += getClassInstanceSize((pLoadData->staticFieldCount - pLoadData->objectStaticFieldCount) + pLoadData->doubleStaticFieldCount, pLoadData->objectStaticFieldCount);
#endif

    pClassDef = (LOADED_CLASS_DEF) pNext;    
    pNext += isInterface ? getLoadedInterfaceStructSize() : getLoadedClassStructSize();

    
    /* methods */
    if(methodCount) {
        pNext = constructMethods(pLoadData, pClassDef, totalInstanceMethodCount, isInterface, pNext);
    }
    else {
        pClassDef->header.pInstanceMethods = NULL;
        pClassDef->header.virtualMethodCount = 0;
        if(!isInterface) {
            pClassDef->loadedHeader.pStaticMethods = NULL;
        }
    }

    /* we cannot bypass the field construction even if there are no fields because the instance
     * size needs to be calculated as well
     */
    pNext = constructFields(pLoadData, pConstantPool, pClassDef, isInterface, pNext);
    
    /* interfaces */
    if(pLoadData->superInterfaceCount) {
        pNext = constructInterfaceList(pLoadData, pClassDef, isInterface, pNext);
    }
    else {
        pClassDef->loadedHeader.pSuperInterfaceList = NULL;
    }
 
    
#if REALLOCATE_CONSTANT_POOL
    pClassDef->loadedHeader.pConstantPool = (CONSTANT_POOL) pNext;
    pNext += getConstantPoolSize(getConstantPoolLength(pConstantPool));
    memoryCopy(pClassDef->loadedHeader.pConstantPool, pConstantPool, getConstantPoolSize(getConstantPoolLength(pConstantPool)));
#else   
    pClassDef->loadedHeader.pConstantPool = pConstantPool;
#endif

    pClassDef->header.key = pLoadData->key;  /* ANSI union assignment */
    pClassDef->header.accessFlags = pLoadData->accessFlags;
    pClassDef->header.pSuperClass = pLoadData->pSuperClass;
    
#if PREPARE_WHILE_LOADING 
    initializeClassInstance(pClassInstance, (COMMON_CLASS_DEF) pClassDef, pLoadData->pMonitor, (BOOLEAN) (pLoadData->staticFieldCount != 0));   
    checkForClassInitializer(pClassInstance, (COMMON_CLASS_DEF) pClassDef);        
    *ppClassInstance = pClassInstance;
#endif

    *ppClassDef = pClassDef;
    ASSERT(pNext == pFirst + size);
   
    return SUCCESS;
}




