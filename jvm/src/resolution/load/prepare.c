
#include "memory/memoryAlloc.h"
#include "javaThread/javaThread.h"
#include "thread/schedule.h"
#include "class/table.h"
#include "prepare.h"


static jstackField nullField;

#if !PREPARE_WHILE_LOADING

static void preparationUpdateClassTable(CLASS_ENTRY pClassEntry, CLASS_INSTANCE pClassInstance) 
{
    CLASS_LOAD_ENTRY pEntry = pClassEntry->u.pLoad;
    ASSERT(pClassEntry->status == CLASS_STATUS_LOADED);

    enterCritical();
    pClassEntry->u.pLoad = NULL;
    pClassEntry->status = CLASS_STATUS_PREPARED;
    pClassEntry->u.pClassInstance = pClassInstance; /* the load structure in the class entry will be garbage collected */
    exitCritical();
    /* we avoid nesting critical regions by freeing the memory outside of the above critical region */
    memoryFree(pEntry);
}

#endif

static void initializeStatics(CLASS_INSTANCE pClassInstance)
{
    UINT16 i;
    CONSTANT_FIELD_DEF pConstantFieldDef;
    LOADED_CLASS_DEF pClassDef = (LOADED_CLASS_DEF) pClassInstance->pRepresentativeClassDef;
    FIELD_LIST pList = pClassDef->loadedHeader.pStaticFields;
    JSTACK_FIELD value1, value2;

    for(i=0; i<pList->length; i++) {
        pConstantFieldDef = (CONSTANT_FIELD_DEF) pList->ppFields[i];
        
        if(isConstantField(pConstantFieldDef)) {
            value1 = &pConstantFieldDef->value;
            value2 = &pConstantFieldDef->value2;
        }
        else {
            value1 = value2 = &nullField;
        }
        if(isPrimitiveField(pConstantFieldDef)) {
            if(isDoubleField(pConstantFieldDef)) {
                jlong value;
            
                setJlong(value, getStackFieldInt(value2), getStackFieldInt(value1));
                setPrimitiveFieldLong(getPrimitiveStaticField(pClassInstance, pConstantFieldDef->fieldIndex), value);
            }
            else {
                setPrimitiveFieldData(getPrimitiveStaticField(pClassInstance, pConstantFieldDef->fieldIndex),  getPrimitiveFieldData((JPRIMITIVE_FIELD) value1));
            }
        }
        else {
            setObjectFieldObject(getObjectStaticField(pClassInstance, pConstantFieldDef->fieldIndex), getStackFieldObject(value1));
        }
    }
}

void initializeClassInstance(CLASS_INSTANCE pClassInstance, COMMON_CLASS_DEF pClassDef, MONITOR pMonitor, 
                                    BOOLEAN hasFields)
{
    pClassInstance->pInitializingThread = NULL;
    pClassInstance->pRepresentativeClassDef = pClassDef;
    setClassInitStatus(pClassInstance, CLASS_INIT_STATUS_UNINITIALIZED);
    extractClassObjectFromClassInstance(pClassInstance).pClassDef = (COMMON_CLASS_DEF) pJavaLangClassDef;
    extractClassObjectFromClassInstance(pClassInstance).pClassInstance = pJavaLangClass;
#if KEEP_CLASS_MONITORS
    extractClassObjectFromClassInstance(pClassInstance).pMonitor = pMonitor;
#else
#if PREEMPTIVE_THREADS
    memoryFree(pMonitor);
#endif
    extractClassObjectFromClassInstance(pClassInstance).pMonitor = NULL;
#endif
    if(hasFields) {
        initializeStatics(pClassInstance);
    }
    return;
}

/* if there is not class initializer to be run, then set the class initialization statu to initialized */
void checkForClassInitializer(CLASS_INSTANCE pClassInstance, COMMON_CLASS_DEF pClassDef)
{
    LOADED_CLASS_DEF pSuperClass;

    /* if the super class is initialized and this class has no static initializer, then mark this class as initialized */
    pSuperClass = pClassDef->pSuperClass;
    if(pSuperClass == NULL) {
        if((pClassDef->accessFlags & ACC_INIT) == 0) {
            setClassInitStatus(pClassInstance, CLASS_INIT_STATUS_INITIALIZED);
        }
    }
    else if(classIsInitialized(getClassInstanceFromClassDef(pSuperClass), getCurrentJavaThread())
            && (pClassDef->accessFlags & ACC_INIT) == 0) {
        setClassInitStatus(pClassInstance, CLASS_INIT_STATUS_INITIALIZED);
    }
}

#if !PREPARE_WHILE_LOADING 

static RETURN_CODE prepareClassInstance(COMMON_CLASS_DEF pClassDef, CLASS_INSTANCE *ppClassInstance)
{
    CLASS_ENTRY pClassEntry;
    CLASS_INSTANCE pClassInstance;
    UINT32 blockSize;
    UINT16 accessFlags = pClassDef->accessFlags;
    UINT16 numPrimitiveSlots = 0;
    UINT16 numObjectSlots = 0;
    RETURN_CODE ret;
#if PREEMPTIVE_THREADS
    MONITOR pMonitor;
#endif

    if((ret = getClassInTable(pClassDef->key.namePackageKey, &pClassEntry)) != SUCCESS) {
        return ret;
    }
    if(pClassEntry->status < CLASS_STATUS_LOADED) {
        return pClassEntry->u.error;
    }

#if PREEMPTIVE_THREADS
    /* if the class was loaded and not romized the monitor will be already there */
    if(pClassEntry->u.pLoad->pMonitor == NULL && (ret = memoryCallocMonitor(&pClassEntry->u.pLoad->pMonitor)) != SUCCESS) {
        return ret;
    }
    
    pMonitor = pClassEntry->u.pLoad->pMonitor;
    monitorEnter(pMonitor);
#endif
    /* Now that we own the monitor, check the status */
    if(pClassEntry->status < CLASS_STATUS_LOADED) {
        ret = pClassEntry->u.error;
    }
    else if(pClassEntry->status == CLASS_STATUS_LOADED) {
        if(isArrayClass(pClassDef)) {
            blockSize = getClassInstanceSize(0, 0);
        }
        else {
            FIELD_LIST pStaticList = getStaticFieldList(pClassDef);
            if(pStaticList == NULL) {
                blockSize = getClassInstanceSize(0, 0);
            }
            else {
                UINT16 i;
                FIELD_DEF pField;
                UINT16 length = pStaticList->length;
                FIELD_DEF *pFields = pStaticList->ppFields;

                for(i=0; i<length; i++) {
                    pField = pFields[i];
                    if(isDoubleField(pField)) {
                        numPrimitiveSlots += 2;
                    }
                    else if(isPrimitiveField(pField)) {
                        numPrimitiveSlots++;
                    }
                    else numObjectSlots++;
                }
                blockSize = getClassInstanceSize(numPrimitiveSlots, numObjectSlots);
            }
        }
        pClassInstance = (CLASS_INSTANCE) memoryAlloc(blockSize); 
        if(pClassInstance == NULL) {
            ret = ERROR_CODE_OUT_OF_MEMORY;
        }
        else {            
            initializeClassInstance(pClassInstance, pClassDef, pMonitor, (BOOLEAN) (numPrimitiveSlots + numObjectSlots != 0));
            checkForClassInitializer(pClassInstance, pClassDef);
            preparationUpdateClassTable(pClassEntry, pClassInstance);
        }
    }
    else {
        pClassInstance = pClassEntry->u.pClassInstance;
    }
    *ppClassInstance = pClassInstance;
#if PREEMPTIVE_THREADS
    monitorExit(pMonitor);
#endif
    return ret;
}

static RETURN_CODE iterativePrepareSuperClasses(CLASS_ENTRY pClassEntry, COMMON_CLASS_DEF pClassDef) 
{
    RETURN_CODE ret;
    COMMON_CLASS_DEF pCurrentClassDef;
    COMMON_CLASS_DEF pSuperClassDef;
    CLASS_INSTANCE pClassInstance;
    
    /* we must prepare the super class of any class first */
    
    /* There are several reasons for this.
     * A separate thread might access the same child class just
     * after a child class is prepared, and seeing that the child class is prepared
     * the second thread will mistakenly assume all parents are prepared, while
     * that may not yet be completed by the first thread.
     * Also, if preparation fails on a parent class then a child class will mistakenly
     * already be marked prepared.
     */

    do {
        pCurrentClassDef = pClassDef;
        pSuperClassDef = (COMMON_CLASS_DEF) getParent(pCurrentClassDef);
        while (pSuperClassDef != NULL && !classInTableIsPrepared(getClassEntryFromClassDef(pSuperClassDef))) {
            pCurrentClassDef = pSuperClassDef;
            pSuperClassDef = (COMMON_CLASS_DEF) getParent(pCurrentClassDef);
        }
        ret = prepareClassInstance(pCurrentClassDef, &pClassInstance);
        if(ret != SUCCESS) {
            return ret;
        }   
    } while(!classInTableIsPrepared(pClassEntry));
    
    return SUCCESS;
}

static RETURN_CODE iterativePrepareArrayClasses(CLASS_ENTRY pClassEntry, COMMON_CLASS_DEF pClassDef)
{
    CLASS_INSTANCE pClassInstance;
    ARRAY_CLASS_DEF pArrayClassDef;
    COMMON_CLASS_DEF pElementClassDef;
    CLASS_ENTRY pElementClassEntry;
    RETURN_CODE ret;

     /* we can assume that the super class of any array class, java.lang.Object, is already prepared
      * because it would have to be in order to enter the very first class
      */

    /* we must also prepare the element class of any array class first, for the same reasons
     * we must prepare the super class of any class first (see above)
     */
    
    if(isPrimitiveArrayClass(pClassDef)) {
        return prepareClassInstance(pClassDef, &pClassInstance);
    }
    else {
        do {
            pArrayClassDef = (ARRAY_CLASS_DEF) pClassDef;
            pElementClassDef = getElementClass(pArrayClassDef);
            pElementClassEntry = getClassEntryFromClassDef(pElementClassDef);
            
            while(!classInTableIsPrepared(pElementClassEntry)) {

                if(!isArrayClass(pElementClassDef)) {
                    ret = iterativePrepareSuperClasses(pElementClassEntry, pElementClassDef);
                    if(ret != SUCCESS) {
                        return ret;
                    }
                    break;
                }
                else { 
                    pArrayClassDef = (ARRAY_CLASS_DEF) pElementClassDef;
                    if(isPrimitiveArrayClass(pElementClassDef)) {
                        break;
                    }
                    pElementClassDef = getElementClass(pArrayClassDef);
                    pElementClassEntry = getClassEntryFromClassDef(pElementClassDef);
                }
            }

            ret = prepareClassInstance((COMMON_CLASS_DEF) pArrayClassDef, &pClassInstance);
            if(ret != SUCCESS) {
                return ret;
            }

        } while(!classInTableIsPrepared(pClassEntry));
    }
    return SUCCESS;
}

RETURN_CODE iterativePrepareClass(CLASS_ENTRY pClassEntry)
{
    COMMON_CLASS_DEF pClassDef = getClassDefFromClassEntry(pClassEntry);

    if(isArrayClass(pClassDef)) {
        return iterativePrepareArrayClasses(pClassEntry, pClassDef);
    }
    else {
        return iterativePrepareSuperClasses(pClassEntry, pClassDef);
    }
}



#endif