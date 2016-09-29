
#include "memory/memoryAlloc.h"
#include "elements/base.h"
#include "interpret/javaGlobals.h"
#include "object/instantiate.h"
#include "log/logClass.h"
#include "log/logItem.h"
#include "thread/thread.h"
#include "thread/schedule.h"
#include "class/table.h"
#include "preload.h"
#include "load.h"
#include "prepare.h"
#include "iterativeLoad.h"


/* a list of classes that require loading */
typedef struct classLoadList {
    UINT16 length;
    UINT16 numClasses;
    PRELOAD_DATA loadList[];
} classLoadList, *CLASSLOADLIST;

#define LOAD_LIST_BLOCK_SIZE 10 /* size of increments in list length */
#define getClassLoadListSize(n) (structOffsetOf(classLoadList, loadList) + sizeof(PRELOAD_DATA) * (n))
#define getClassFromList(pClassList, index) ((pClassList)->loadList[index])



static void releaseIterativeLoadStructure(PRELOAD_DATA pPreloadData)
{
    if(pPreloadData->pSuperInterfaces != NULL) {
        memoryFree(pPreloadData->pSuperInterfaces);
        pPreloadData->pSuperInterfaces = NULL;
    }
    if(pPreloadData->pFileAccess != NULL) {
        pPreloadData->pFileAccess->pFileAccessMethods->close(pPreloadData->pFileAccess);
        pPreloadData->pFileAccess = NULL;
    }
    if(pPreloadData->pFirstConstantPool != NULL) {
        FIRST_PASS_CONSTANT_POOL pFirstConstantPool = pPreloadData->pFirstConstantPool;
        UINT32 i, length = getConstantPoolLength(pFirstConstantPool);

        for(i=1; i<length; i++) {
            if(getConstantPoolTag(pFirstConstantPool, i) == CONSTANT_Utf8) {
                memoryFree(pFirstConstantPool[i].pUtf8String);
            }
        }
        memoryFree(pFirstConstantPool);
        pPreloadData->pFirstConstantPool = NULL;
    }
    if(pPreloadData->pConstantPool != NULL) {
        memoryFree(pPreloadData->pConstantPool);
        pPreloadData->pConstantPool = NULL;
    }

#if PREEMPTIVE_THREADS
    if(pPreloadData->pMonitor != NULL) {
        monitorExit(pPreloadData->pMonitor);
        pPreloadData->pMonitor = NULL;
    }
#endif

}

static UINT16 removeClassFromList(CLASSLOADLIST pClassList, UINT16 index)
{
    UINT16 classesAbove = pClassList->numClasses - index - 1;
    
    releaseIterativeLoadStructure(pClassList->loadList[index]);
    memoryFree(pClassList->loadList[index]);

    /* move everything past the removed entry down one in the list */
    if(classesAbove) {
        memoryMove( (pClassList->loadList) + index, (pClassList->loadList) + index + 1, 
            sizeof(PRELOAD_DATA) * classesAbove);
    }
    (pClassList->numClasses)--;
    return TRUE;
}

static void removeDuplicates(CLASSLOADLIST pClassList)
{
    UINT16 i, j;
    
    for(i=1; i<pClassList->numClasses; i++) {
        for(j=i+1; j<pClassList->numClasses; j++) {
            if(getClassFromList(pClassList, pClassList->numClasses - i - 1)->classID.key.namePackageKey == 
                    getClassFromList(pClassList, pClassList->numClasses - j - 1)->classID.key.namePackageKey) {
                removeClassFromList(pClassList, (UINT16) (pClassList->numClasses - j - 1));
                ++i;
            }       
        }
    }
}

static BOOLEAN addClassToList(CLASSLOADLIST *ppClassList, PRELOAD_DATA pPreloadData)
{
    /* allocate more space if necessary -  this will usually not be necessary */
    if((*ppClassList)->length == (*ppClassList)->numClasses)
    {
        void *tmp;
        UINT16 newSize = getClassLoadListSize((*ppClassList)->length + LOAD_LIST_BLOCK_SIZE);

         /* determine the new list length */
        tmp = memoryAlloc(newSize);
        if (tmp == NULL) {
            return FALSE;
        }
        memoryCopy(tmp, *ppClassList, getClassLoadListSize((*ppClassList)->length));
        memoryFree(*ppClassList);
        *ppClassList = tmp;
        (*ppClassList)->length = newSize;
    }
    (*ppClassList)->loadList[(*ppClassList)->numClasses] = pPreloadData;
    ((*ppClassList)->numClasses)++;
    return TRUE;
}

static BOOLEAN foundCircularity(CLASSLOADLIST pClassList, PRELOAD_DATA pClassLoadData)
{
    NamePackageKey key = pClassLoadData->classID.key.namePackageKey;
    PRELOAD_DATA pChildLoad = pClassLoadData->pChildLoad;

    while(pChildLoad != NULL) {
        /* if our class matches its own child, we have a circularity */
        if(key == pChildLoad->classID.key.namePackageKey) {
            return TRUE;
        }
        pChildLoad = pChildLoad->pChildLoad;
    }
    return FALSE;
}

static PRELOAD_DATA createPreloadStructure(CLASS_ID pClass, PRELOAD_DATA pChildLoadData)
{
    PRELOAD_DATA pPreloadData;
    
    pPreloadData = memoryAlloc(sizeof(preloadDataStruct));
    if(pPreloadData == NULL) {
        return NULL;
    }
    pPreloadData->classID = *pClass; /* ANSI structure assignment */
    pPreloadData->pChildLoad = pChildLoadData;

    /* Any member that may be explicitly released by releaseIterativeLoadStructure
     * must be set to NULL here 
     */
    pPreloadData->pSuperInterfaces = NULL;
#if PREEMPTIVE_THREADS
    pPreloadData->pMonitor = NULL;
#endif
    pPreloadData->pFileAccess = NULL;
    pPreloadData->pFirstConstantPool = NULL;
    pPreloadData->pConstantPool = NULL;
    
    return pPreloadData;
}

static RETURN_CODE addToLoadList(CLASSLOADLIST *ppClassList, CLASS_ID pClassID, PRELOAD_DATA pChildLoadData)
{
    PRELOAD_DATA pPreloadData;
    
    pPreloadData = createPreloadStructure(pClassID, pChildLoadData);
    if(pPreloadData == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }

    if(foundCircularity(*ppClassList, pPreloadData)) {
        return ERROR_CODE_CIRCULARITY;
    }
    if(!addClassToList(ppClassList, pPreloadData)) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    return SUCCESS;
}

static RETURN_CODE addParentsToLoadList(CLASSLOADLIST *ppClassList, PRELOAD_DATA pChildLoadData)
{
    UINT16 interfaceCount = pChildLoadData->superInterfaceCount;
    UINT16 i;
    RETURN_CODE ret;
    CLASS_ENTRY pClassEntry;

    if(pChildLoadData->superClassID.nameLength != 0) {
        pClassEntry = NULL;
        if(!classIsLoaded(pChildLoadData->superClassID.key.namePackageKey, &pClassEntry)) {
            if(pClassEntry && classInTableIsInError(pClassEntry)) {
                return pClassEntry->u.error;
            }
            ret = addToLoadList(ppClassList, &pChildLoadData->superClassID, pChildLoadData);
            if(ret != SUCCESS) {
                return ret;
            }
        }
        
    }
    if(pChildLoadData->superInterfaceCount > 0) {
        for(i=0; i<interfaceCount; i++) {
            pClassEntry = NULL;
            if(!classIsLoaded(pChildLoadData->pSuperInterfaces[i].key.namePackageKey, &pClassEntry)) {
                if(pClassEntry && classInTableIsInError(pClassEntry)) {
                    return pClassEntry->u.error;
                }
                ret = addToLoadList(ppClassList, pChildLoadData->pSuperInterfaces + i, pChildLoadData);
                if(ret != SUCCESS) {
                    return ret;
                }
            }
        }
    }
    return SUCCESS;
}

static void releaseIterativeLoadResources(CLASSLOADLIST pClassList)
{
    UINT16 i;
    PRELOAD_DATA pPreloadData;

    /* free up allocated memory, close open files, and exit monitors */

    for(i=0; i<pClassList->numClasses; i++) {
        pPreloadData = pClassList->loadList[i];  
        releaseIterativeLoadStructure(pPreloadData);
        memoryFree(pPreloadData);
    }
    memoryFree(pClassList);
}

static void setChildStatus(PRELOAD_DATA pPreloadData, RETURN_CODE ret)
{
    PRELOAD_DATA pChildLoadData = pPreloadData;

    /* disable interrupts while we alter the contents of the class table */
    enterCritical();

    do {
        pChildLoadData->pClassEntry->u.error = ret;
        pChildLoadData->pClassEntry->status = CLASS_STATUS_ERROR;
        pChildLoadData = pChildLoadData->pChildLoad;
    } while(pChildLoadData != NULL);

    exitCritical();
}

static void updateClassTable(CLASS_ENTRY pClassEntry, CLASS_INSTANCE pClassInstance, COMMON_CLASS_DEF pClassDef)
{
    /* we must disable the interrupts while we alter the class table */
#if PREPARE_WHILE_LOADING
    enterCritical();
    pClassEntry->status = CLASS_STATUS_PREPARED;
    pClassEntry->u.pClassInstance = pClassInstance;
    exitCritical();
#else
    CLASS_LOAD_ENTRY pLoadEntry = memoryAlloc(sizeof(classLoadEntryStruct));
    
    enterCritical();
    pClassEntry->status = CLASS_STATUS_LOADED;
    pLoadEntry->pMonitor = pClassEntry->u.pMonitor;
    pLoadEntry->pClassDef = pClassDef;
    pClassEntry->u.pLoad = pLoadEntry;
    exitCritical();
#endif   

}



static RETURN_CODE constructArrayClass(NamePackage key, CLASS_INSTANCE *ppClassInstance, ARRAY_CLASS_DEF *ppClassDef, 
                                       BOOLEAN isPrimitive, COMMON_CLASS_DEF pElementClassDef, ARRAY_TYPE primitiveType,
                                       MONITOR pMonitor)
{
    ARRAY_CLASS_DEF pClassDef;
    COMMON_CLASS_DEF pHeader;
    PBYTE pMem;
    
#if PREPARE_WHILE_LOADING
    CLASS_INSTANCE pClassInstance;
    UINT32 size = getArrayClassStructSize() + getClassInstanceSize(0, 0);
#else
    UINT32 size = getArrayClassStructSize();
#endif
    
    pMem = memoryAlloc(size);
    if(pMem == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    /* for the sake of the garbage collector the class object must appear at the top of its block,
     * so that the block header for class objects is located just above the object, just like
     * all other objects
     */
#if PREPARE_WHILE_LOADING
    pClassInstance = (CLASS_INSTANCE) pMem;
    pMem += getClassInstanceSize(0, 0);
#endif
    pClassDef = (ARRAY_CLASS_DEF) pMem;
    pHeader = (COMMON_CLASS_DEF) pClassDef;
    pHeader->key = key;  /* ANSI union assignment */
    if(isPrimitive) {
        pHeader->accessFlags = ACC_PUBLIC_FINAL_SUPER;
        pClassDef->elementClass.primitiveType = primitiveType;
        switch(primitiveType) {
            case BOOLEAN_ARRAY_TYPE: pClassDef->elementByteSize = sizeof(jboolean); break;
            case CHAR_ARRAY_TYPE: pClassDef->elementByteSize = sizeof(jchar); break;
            case INT_ARRAY_TYPE: pClassDef->elementByteSize = sizeof(jint); break;
            case LONG_ARRAY_TYPE: pClassDef->elementByteSize = sizeof(jlong); break;
            case SHORT_ARRAY_TYPE: pClassDef->elementByteSize = sizeof(jshort); break;
            case BYTE_ARRAY_TYPE: pClassDef->elementByteSize = sizeof(jbyte); break;
#if IMPLEMENTS_FLOAT
            case FLOAT_ARRAY_TYPE: pClassDef->elementByteSize = sizeof(jfloat); break;
            case DOUBLE_ARRAY_TYPE: pClassDef->elementByteSize = sizeof(jdouble); break;
#endif
            /* we should never arrive here */
            default: return ERROR_CODE_INVALID_ARRAY_TYPE;
        }
    }
    else {
        /* an array class requires initialization if its element class does, same goes for whether it is considered public */
        pHeader->accessFlags = ACC_FINAL_SUPER | (pElementClassDef->accessFlags & ACC_PUBLIC_INIT);
        pClassDef->elementClass.pElementClass = pElementClassDef;
        pClassDef->elementByteSize = getObjectFieldSize();
    }
    pHeader->pSuperClass = pJavaLangObjectDef;
    pHeader->virtualMethodCount = pJavaLangObjectDef->header.virtualMethodCount;
    pHeader->pInstanceMethods = getInstanceMethodList(pJavaLangObjectDef);
    pHeader->objectInstanceFieldCount = 0;
#if PREPARE_WHILE_LOADING 
    initializeClassInstance(pClassInstance, (COMMON_CLASS_DEF) pClassDef, pMonitor, FALSE);
    setClassInitStatus(pClassInstance, CLASS_INIT_STATUS_INITIALIZED); /* array classes do not have class initializers (<clinit> methods) */
    *ppClassInstance = pClassInstance;
#endif

    *ppClassDef = pClassDef;
    return SUCCESS;
}




RETURN_CODE loadArrayClass(NamePackage classKey, BOOLEAN isPrimitiveElementClass, ARRAY_TYPE primitiveType, 
                           COMMON_CLASS_DEF pElementClassDef, RETURN_CODE elementClassResult, 
                           ARRAY_CLASS_DEF *ppClassDef, CLASS_ENTRY *ppClassEntry)
{
    RETURN_CODE ret;
    CLASS_ENTRY pClassEntry;
        
    /* reserve a class table entry if one is not yet already reserved */
    if((ret = getClassInTable(classKey.namePackageKey, &pClassEntry)) != SUCCESS) {
        return ret;
    }
        
    /* we must enter the monitor in order to do any loading */
    if(!classInTableIsLoaded(pClassEntry)) {

        ARRAY_CLASS_DEF pClassDef;
        CLASS_INSTANCE pClassInstance;
        MONITOR pMonitor;
        
#if PREEMPTIVE_THREADS
        MONITOR *ppMonitor = getClassMonitorPtrFromEntry(pClassEntry);

        if(*ppMonitor != NULL || (ret = memoryCallocMonitor(ppMonitor)) == SUCCESS) {
            pMonitor = *ppMonitor;
            monitorEnter(pMonitor);
#else
            pMonitor = NULL;
#endif
            if(classInTableIsLoaded(pClassEntry)) {          
                /* some other thread loaded this class */
                ret = SUCCESS;
            }
            else if(classInTableIsInError(pClassEntry)) {
                /* some other thread has failed to load this class */
                ret = pClassEntry->u.error;
            }
            else if(elementClassResult != SUCCESS) {
                /* don't bother even trying if the element class load failed */
                ret = pClassEntry->u.error = elementClassResult;
                pClassEntry->status = CLASS_STATUS_ERROR;
            }
            else {
                /* we have the monitor and the go-ahead to load */
                ret = constructArrayClass(classKey, &pClassInstance, &pClassDef, isPrimitiveElementClass, pElementClassDef, primitiveType, pMonitor); 
                if(ret == SUCCESS) {
                    updateClassTable(pClassEntry, pClassInstance, (COMMON_CLASS_DEF) pClassDef);
                    pClassDef->header.tableIndex = getClassTableIndex(getClassTable(), pClassEntry);
                    *ppClassDef = pClassDef;
                }

                LOG_CLASS_REF("Loaded array class", classKey);
                LOG_CLASS(pClassDef);
            }
#if PREEMPTIVE_THREADS
            monitorExit(*ppMonitor);
        }
#endif
    }
    *ppClassEntry = pClassEntry;
    return ret;
}


static RETURN_CODE iterativeLoadArrayClass(CLASS_ID pClassID, UINT16 dimensions, CLASS_ENTRY *ppClassEntry)
{
    const char *name = pClassID->name;
    RETURN_CODE elementClassStatus;
    COMMON_CLASS_DEF pElementClassDef = NULL;
    ARRAY_CLASS_DEF pClassDef;
    BOOLEAN isPrimitiveElementClass;
    ARRAY_TYPE primitiveType = BOOLEAN_ARRAY_TYPE;
    CLASS_ENTRY pClassEntry;
    UINT16 currentDimension = 1;
    NamePackage arrayNamePackage = pClassID->key;  /* ANSI union assignment */
    
    

    if(isPrimitiveArrayPackageKey(arrayNamePackage.np.packageKey)) {
                
        switch(name[0]) {
            /* no need to load primitives */
            case BOOLEAN_ARRAY_TYPE:
            case CHAR_ARRAY_TYPE:
            case INT_ARRAY_TYPE:
            case LONG_ARRAY_TYPE:
            case SHORT_ARRAY_TYPE:
            case BYTE_ARRAY_TYPE:
#if IMPLEMENTS_FLOAT
            case DOUBLE_ARRAY_TYPE:
            case FLOAT_ARRAY_TYPE:
#endif
                primitiveType = name[0];
                elementClassStatus = SUCCESS;
                isPrimitiveElementClass = TRUE;
                break;

            
            default:
                return ERROR_CODE_INVALID_ARRAY_TYPE;
        }
    }
    else { /* load the element type of the array */

        isPrimitiveElementClass = FALSE;
		 
        /* set the array depth to 0 in the key */
        pClassID->key.np.packageKey = addArrayDepthToPackageKey(getPackageStringKey(arrayNamePackage.np.packageKey), 0); 
        
        /* ensure the element class is loaded */
        if(classIsLoaded(pClassID->key.namePackageKey, &pClassEntry)) {
            pElementClassDef = getClassDefFromClassEntry(pClassEntry);
            elementClassStatus = SUCCESS;
        }
        else if(pClassEntry && classInTableIsInError(pClassEntry)) {
            elementClassStatus = pClassEntry->u.error;
        }
        else {
            elementClassStatus = iterativeLoadClass(pClassID, &pClassEntry);
            if(elementClassStatus == SUCCESS) {
                pElementClassDef = getClassDefFromClassEntry(pClassEntry);
            }
        }

        /* reset the original key back to its original value */
        pClassID->key.np.packageKey = arrayNamePackage.np.packageKey;
    }

    /* load the array classes in the correct order by dimension */
    while(currentDimension <= dimensions) {

        /* alter the package key to reflect the correct array class dimension */
        arrayNamePackage.np.packageKey = addArrayDepthToPackageKey(getPackageStringAndPrimitiveBitKey(arrayNamePackage.np.packageKey), currentDimension); 
        
        LOG_CLASS_REF("Loading array class", arrayNamePackage);

        /* load the array class */
        elementClassStatus = loadArrayClass(arrayNamePackage, isPrimitiveElementClass, primitiveType, pElementClassDef, 
            elementClassStatus, (ARRAY_CLASS_DEF *) &pClassDef, ppClassEntry);

        isPrimitiveElementClass = FALSE;
        pElementClassDef = (COMMON_CLASS_DEF) pClassDef;    
        currentDimension++;
    }

    
    return elementClassStatus;
}


RETURN_CODE loadClass(NamePackage classKey, CLASS_ENTRY *ppClassEntry)
{
    classIdentifierStruct id;
    UINT16 arrayDimensions;
    RETURN_CODE ret;

    id.key = classKey;  /* ANSI union assignment */
    id.name = getUTF8StringByKey(classKey.np.nameKey, &id.nameLength);
    id.packageName = getPackageNameStringByKey(classKey.np.packageKey, &id.packageNameLength);
    arrayDimensions = getArrayDepth(classKey.np.packageKey);
    if(arrayDimensions) {
        ret = iterativeLoadArrayClass(&id, arrayDimensions, ppClassEntry);
    }
    else {
        ret = iterativeLoadClass(&id, ppClassEntry);
    }
    return ret;
}


RETURN_CODE iterativeLoadClass(CLASS_ID pClassID, CLASS_ENTRY *ppClassEntry)
{
    UINT16 i;
    UINT16 index = 0;
    CLASSLOADLIST pClassList;
    RETURN_CODE ret;
    PRELOAD_DATA pPreloadData;
    CLASS_ENTRY pClassEntry;
    
    LOG_CLASS_REF("Loading compiled class", pClassID->key);
    
    pClassList = memoryAlloc(getClassLoadListSize(LOAD_LIST_BLOCK_SIZE));
    if(pClassList == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    
    pClassList->length = LOAD_LIST_BLOCK_SIZE;
    pClassList->numClasses = 0;

    
    pPreloadData = createPreloadStructure(pClassID, NULL);
    if(pPreloadData == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    addClassToList(&pClassList, pPreloadData);
         
    /*
     * Iteratively probe all classes and interfaces for parent classes and
     * implemented interfaces yet to be loaded.
     * The loop terminates when every unloaded class in the list has all its
     * unloaded parents later on in the list.
     */
    while(index < pClassList->numClasses) {
        pPreloadData = getClassFromList(pClassList, index);
        
        /* reserve a class table entry if one is not yet already reserved */
        if((ret = getClassInTable(pPreloadData->classID.key.namePackageKey, &pClassEntry)) != SUCCESS) {
            releaseIterativeLoadResources(pClassList);
            return ret;
        }
        pPreloadData->pClassEntry = pClassEntry;
            
        if(!classInTableIsLoaded(pClassEntry)) {

#if PREEMPTIVE_THREADS

            
            MONITOR *ppMonitor = getClassMonitorPtrFromEntry(pClassEntry);
            MONITOR pMonitor;

            if(*ppMonitor == NULL && (ret = memoryCallocMonitor(ppMonitor)) != SUCCESS) {
                releaseIterativeLoadResources(pClassList);
                return ret;
            }
            pPreloadData->pMonitor = pMonitor = *ppMonitor;
            monitorEnter(pMonitor);

            
#endif
            if(!classInTableIsLoaded(pClassEntry)) {
                if(classInTableIsInError(pClassEntry)) {
                    /* some other thread failed to load the class */
                    /* it is important that we release the resources after we set the child status since we own the monitor */
                    setChildStatus(pPreloadData, pClassEntry->status);
                    releaseIterativeLoadResources(pClassList);
                    return pClassEntry->u.error;
                }
                else {
                    /* we have the monitor and the go-ahead to load the class */
                    if((ret = preloadClass(pPreloadData)) != SUCCESS) {
                        setChildStatus(pPreloadData, ret);
                        releaseIterativeLoadResources(pClassList);
                        return ret;
                    }
                    if((ret = addParentsToLoadList(&pClassList, pPreloadData)) != SUCCESS) {
                        releaseIterativeLoadResources(pClassList);
                        return ret;
                    }
                }
                index++; /* examine the next class in the list */
            }
            else {
                removeClassFromList(pClassList, index); /* the class has been loaded in a separate thread */
            }
        }
        else {
            removeClassFromList(pClassList, index); /* the class has been loaded */
        }
    }

    if(pClassList->numClasses > 0) {
        /*
         * remove any duplicates in the list, which may occur if two implemented
         * interfaces inherit from the same interface
         */
        removeDuplicates(pClassList);
    

        /*
         * load all classes in the list in the correct order,
         * the most recently added first
         */
        for(i=0; i<pClassList->numClasses; i++) {

            pPreloadData = getClassFromList(pClassList, pClassList->numClasses - 1 - i);
            ret = completeClassLoad(pPreloadData);
            
            if(ret != SUCCESS) {
                
                /* it is important that we release the resources after we set the child status since we own the monitor */
                setChildStatus(pPreloadData, ret);
                releaseIterativeLoadResources(pClassList);
                return ret;
            }
#if !REALLOCATE_CONSTANT_POOL
            else  {
                /* set the constant pool entry to NULL so the memory is not released by releaseIterativeLoadStructure */
                pPreloadData->pConstantPool = NULL;
            }
#endif

            LOG_CLASS_REF("Loaded compiled class", pPreloadData->classID.key);
            LOG_CLASS(pPreloadData->pClassDef);

            updateClassTable(pPreloadData->pClassEntry, pPreloadData->pClassInstance, (COMMON_CLASS_DEF) pPreloadData->pClassDef);
            releaseIterativeLoadStructure(pPreloadData);
        }
        *ppClassEntry = pPreloadData->pClassEntry;
    }
    else {
        *ppClassEntry = pClassEntry;
    }
    releaseIterativeLoadResources(pClassList);

    return SUCCESS;
}




