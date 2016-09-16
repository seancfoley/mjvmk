
#include "interpret/javaGlobals.h"
#include "object/instantiate.h"
#include "string/stringKeys.h"
#include "thread/thread.h"
#include "class/table.h"
#include "resolution/resolve/lookup.h"
#include "resolution/constantPool.h"
#include "iterativeLoad.h"
#include "prepare.h"
#include "initializeLoader.h"




static void fixInternedStrings(LOADED_CLASS_DEF pClassDef)
{
    CONSTANT_POOL pConstantPool = pClassDef->loadedHeader.pConstantPool;
    INTERNED_STRING_INSTANCE pString;
    UINT32 i;

    for(i=1; i<getConstantPoolLength(pConstantPool); i++) {
        if(getConstantPoolUnmaskedTag(pConstantPool, i) == CONSTANT_String) {
            pString = getConstantPoolInternedString(pConstantPool, i);
            extractObjectFieldObject(pString->pCharArray)->header.pClassDef = (COMMON_CLASS_DEF) pCharArrayClassDef;
            extractObjectFieldObject(pString->pCharArray)->header.pClassInstance = pCharArrayClass;
            pString->header.pClassDef = (COMMON_CLASS_DEF) pJavaLangStringDef;
            pString->header.pClassInstance = pJavaLangString;
        }
    }
}

static void fixClassInstance(CLASS_INSTANCE pClassInstance)
{
    extractClassObjectFromClassInstance(pClassInstance).pClassDef = (COMMON_CLASS_DEF) pJavaLangClassDef;
    extractClassObjectFromClassInstance(pClassInstance).pClassInstance = pJavaLangClass;
}


#define handleOperation(op) if((ret = (op)) != SUCCESS) return ret
#define loadGlobalClass(key, ppClassEntry) handleOperation(loadClass(key, ppClassEntry))
#define instantiateGlobalObject(pClassInstance, ppObjectInstance) handleOperation(instantiatePermanentObject(pClassInstance, ppObjectInstance))

#if PREPARE_WHILE_LOADING
#define obtainClassInstance(pClassEntry, ppClassInstance) (*(ppClassInstance) = (pClassEntry)->u.pClassInstance)
#else
#define obtainClassInstance(pClassEntry, ppClassInstance) handleOperation(iterativePrepareClass(pClassEntry)); *(ppClassInstance) = (pClassEntry)->u.pClassInstance
#endif


RETURN_CODE initializeLoader()
{
    RETURN_CODE ret;
    CLASS_ENTRY pClassEntry, pClassEntry2;
    CLASS_INSTANCE pClassInstance;
    
    /* There are a couple of tricky initialization issues here.  The first is that when loading
     * java/lang/String and java/lang/Object, any interned strings will not correctly point to 
     * java/lang/String yet because it is not yet loaded.
     * So we need to load these two first and then fix their interned strings after.
     * 
     * At that point it is safe to load any other class without any interned string problems.
     * 
     * The second issue is that when preparing java/lang/Object, java/lang/String and
     * java/lang/Class, their associated class instances will not correctly point to 
     * java/lang/Class because it hasn't been loaded yet.  So we must go in and fix this 
     * afterwards.
     *
     * Once this is done, it is safe to load any other class without any problems at all.
     */

    /* first we load String and Object */
    loadGlobalClass(javaLangStringKey, &pClassEntry);
    
    handleOperation(getClassInTable(javaLangObjectKey.namePackageKey, &pClassEntry2));    
    obtainClassInstance(pClassEntry2, &pJavaLangObject);
    obtainClassInstance(pClassEntry, &pJavaLangString);
    pJavaLangStringDef = (LOADED_CLASS_DEF) (pJavaLangString->pRepresentativeClassDef);
    pJavaLangObjectDef = (LOADED_CLASS_DEF) (pJavaLangObject->pRepresentativeClassDef);

    /* now we load the character array class */
    loadGlobalClass(charArrayKey, &pClassEntry);
    obtainClassInstance(pClassEntry, &pCharArrayClass);
    pCharArrayClassDef = (ARRAY_CLASS_DEF) (pCharArrayClass->pRepresentativeClassDef); 

    /* now we load Class */
    loadGlobalClass(javaLangClassKey, &pClassEntry); 
    obtainClassInstance(pClassEntry, &pJavaLangClass);
    pJavaLangClassDef = (LOADED_CLASS_DEF) (pJavaLangClass->pRepresentativeClassDef);
    
    /* fix the deficiencies in the classes loaded so far.
     * After this point, there will be no deficiencies in any loaded class.
     */
    fixInternedStrings(pJavaLangObjectDef);
    fixInternedStrings(pJavaLangStringDef);
    fixClassInstance(pJavaLangString);
    fixClassInstance(pJavaLangObject);
    fixClassInstance(pJavaLangClass);


    /* now we load the thread class */
    loadGlobalClass(javaLangThreadKey, &pClassEntry); 
    obtainClassInstance(pClassEntry, &pJavaLangThread);
    pJavaLangThreadDef = (LOADED_CLASS_DEF) (pJavaLangThread->pRepresentativeClassDef);

    /* We will need the out of memory error for the event that we run out of memory */
    loadGlobalClass(javaLangOutOfMemoryErrorKey, &pClassEntry);
    obtainClassInstance(pClassEntry, &pClassInstance);
    instantiateGlobalObject(pClassInstance, &pJavaLangOutOfMemoryErrorInstance);

    /* if we are growing stack, then we will need the StackOverflowError for the same reason as the out of memory error */
    loadGlobalClass(javaLangStackOverflowErrorKey, &pClassEntry);
    obtainClassInstance(pClassEntry, &pClassInstance);
    instantiateGlobalObject(pClassInstance, &pJavaLangStackOverflowErrorInstance);

    /* load the stack trace class, needed whenever something is thrown */
    loadGlobalClass(comMjvmkStackTraceKey, &pClassEntry); 
    obtainClassInstance(pClassEntry, &pComMjvmkStackTrace);
    
    /* now we load the integer array and the short array class, needed to construct stack traces */
    loadGlobalClass(integerArrayKey, &pClassEntry);
    obtainClassInstance(pClassEntry, &pIntegerArrayClass);
    
    loadGlobalClass(shortArrayKey, &pClassEntry);
    obtainClassInstance(pClassEntry, &pShortArrayClass);
    
    /* we keep an Error instance as a backup in case we are unable to create some other Error in the throwing process */
    handleOperation(getClassInTable(javaLangErrorKey.namePackageKey, &pClassEntry));
    obtainClassInstance(pClassEntry, &pClassInstance);
    instantiateGlobalObject(pClassInstance, &pJavaLangErrorInstance);

    /* we need javaLangIllegalMonitorStateExceptionInstance in case an illegal monitor state occurs
     * while throwing OutOfMemoryError (not likely, but it can happen :-) ) 
     */
    loadGlobalClass(javaLangIllegalMonitorStateExceptionKey, &pClassEntry);
    obtainClassInstance(pClassEntry, &pClassInstance);
    instantiateGlobalObject(pClassInstance, &pJavaLangIllegalMonitorStateExceptionInstance);

    /* the String array class is needed to create the argument to the start method void main(String argv[]) */
    loadGlobalClass(javaLangStringArrayKey, &pClassEntry);
    obtainClassInstance(pClassEntry, &pJavaLangStringArrayClass);

    return ret;
}

