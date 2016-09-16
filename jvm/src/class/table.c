
#include "memory/memoryAlloc.h"
#include "interpret/javaGlobals.h"
#include "table.h"

/* 5.3.5 of JVMS describes the loading procedure */
/* 12.4.2 of JLS or 2.17.5 of JVMS specifies locking required for initialization */


RETURN_CODE initializeClassTable() 
{
    CLASS_TABLE classTable;

   
    /* the first two elements are the class table count and size (2 bytes each) */
    classTable = memoryCalloc(calculateClassTableSize(), 1);
    if(classTable == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    setClassTable(classTable);
    setClassTableSize(classTable, CLASS_TABLE_SIZE);
    return SUCCESS;
}


#define getHashIndex(key, classTableSize) ((UINT16) (key % classTableSize))

BOOLEAN classIsInTableGeneric(CLASS_ENTRY classTableBase, NamePackageKey key, UINT16 classTableSize, CLASS_ENTRY *ppClassEntry)
{
    UINT16 hashedIndex = getHashIndex(key, classTableSize);
    UINT16 currentIndex = hashedIndex;
    CLASS_ENTRY slot = classTableBase + hashedIndex;

    while(slot->status != CLASS_STATUS_INEXISTENT) {
        if(slot->key == key) {
            *ppClassEntry = slot;
            return TRUE;
        }
        currentIndex = getHashIndex(++currentIndex, classTableSize);
        if(currentIndex == hashedIndex) {
            *ppClassEntry = NULL;
            return FALSE;
        }
        slot = classTableBase + currentIndex; 
    }
    *ppClassEntry = NULL;
    return FALSE;
}

static RETURN_CODE findClassTableIndex(CLASS_ENTRY classTableBase, NamePackageKey key, UINT16 classTableSize, UINT16 *actualIndex, UINT16 hashedIndex)
{
    UINT16 indexStart = *actualIndex;
    CLASS_ENTRY slot = classTableBase + indexStart;

    while(slot->status != CLASS_STATUS_INEXISTENT) {
        if(slot->key == key) {
            *actualIndex = indexStart;
            return SUCCESS;
        }
        indexStart = (++indexStart) % classTableSize;
        if(indexStart == hashedIndex) {
            return ERROR_CODE_CLASS_TABLE_FULL;
        }
        slot = classTableBase + indexStart; 
    }
    *actualIndex = indexStart;
    return ERROR_CODE_NO_CLASS_DEF_FOUND;
}


/* finds the class entry corresponding to the indicated class.
   If it is not found, will point to the first available slot for that class */
RETURN_CODE getClassInTableGeneric(CLASS_ENTRY classTableBase, NamePackageKey key, UINT16 classTableSize, CLASS_ENTRY *ppClassEntry) 
{
    UINT16 hashIndex, actualIndex;
    RETURN_CODE ret;

    actualIndex = hashIndex = getHashIndex(key, classTableSize);
    
    ret = findClassTableIndex(classTableBase, key, classTableSize, &actualIndex, hashIndex);
    if(ret == SUCCESS) {
        *ppClassEntry = classTableBase + actualIndex;
    }
    else if(ret == ERROR_CODE_CLASS_TABLE_FULL) {
        *ppClassEntry = NULL;
    }
    else { /* ret == ERROR_CODE_NO_CLASS_DEF_FOUND */
#if PREEMPTIVE_THREADS

        /* just in case another thread is putting stuff in the class table, we try again inside
         * a critical region, but starting from the spot we were previously given. 
         */
        enterCritical();

        ret = findClassTableIndex(classTableBase, key, classTableSize, &actualIndex, actualIndex);
        if(ret == SUCCESS) {
            /* some other thread just inserted the class */
            *ppClassEntry = classTableBase + actualIndex;
        }
        else if(ret == ERROR_CODE_CLASS_TABLE_FULL) {
            *ppClassEntry = NULL;
        }
        else { /* ret == ERROR_CODE_NO_SUCH_CLASS: reserve the slot */
#endif
            CLASS_ENTRY pClassEntry;

            *ppClassEntry = pClassEntry = classTableBase + actualIndex;
            pClassEntry->key = key;
            pClassEntry->status = CLASS_STATUS_REFERENCED;
            incrementClassTableCount(getClassTable());
            ret = SUCCESS;
#if PREEMPTIVE_THREADS           
        }

        exitCritical();
#endif
    }
    return ret;
}

/* 
 * Returns NULL is the class is nowhere to be found.
 */
COMMON_CLASS_DEF getClassDefInTable(NamePackageKey key)
{
    CLASS_ENTRY pClassEntry;
    RETURN_CODE ret;

    if((ret = getClassInTable(key, &pClassEntry)) != SUCCESS) {
        return NULL;
    }
    if(pClassEntry->status < CLASS_STATUS_LOADED) {
        return NULL;
    }
    return getClassDefFromClassEntry(pClassEntry);
}


