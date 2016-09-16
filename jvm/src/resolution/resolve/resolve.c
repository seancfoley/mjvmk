
#include "interpret/javaGlobals.h"
#include "log/logItem.h"
#include "class/table.h"
#include "resolution/load/load.h"
#include "resolution/load/prepare.h"
#include "lookup.h"
#include "resolve.h"


RETURN_CODE obtainClassRef(NamePackage classKey, CLASS_INSTANCE *ppClassInstance)
{
    RETURN_CODE ret;
    CLASS_ENTRY pClassEntry;
    

    ret = getClassInTable(classKey.namePackageKey, &pClassEntry);
    if(ret != SUCCESS) {
        return ret;
    }
    if(!classInTableIsPrepared(pClassEntry)) {
        if(!classInTableIsLoaded(pClassEntry)) {
            if(classInTableIsInError(pClassEntry)) {
                return pClassEntry->u.error;
            }
            ret = loadClass(classKey, &pClassEntry);
            if(ret != SUCCESS) {
                return ret;
            }
        }
       
#if !PREPARE_WHILE_LOADING
        ret = iterativePrepareClass(pClassEntry);
        if(ret != SUCCESS) {
            return ret;
        }
#endif
    }
    *ppClassInstance = pClassEntry->u.pClassInstance;
    return SUCCESS;
}


 
RETURN_CODE resolveClassRef(LOADED_CLASS_DEF pCurrentClass, NamePackage classKey, CLASS_INSTANCE *ppClassInstance)
{
    RETURN_CODE ret;

    LOG_CLASS_REF("Resolving class", classKey);
    
    ret = obtainClassRef(classKey, ppClassInstance);
    
    if(ret != SUCCESS) {
        return ret;
    }

    if(!isArrayClass((*ppClassInstance)->pRepresentativeClassDef) && !isClassAccessible((COMMON_CLASS_DEF) pCurrentClass, (*ppClassInstance)->pRepresentativeClassDef)) {
        return ERROR_CODE_ILLEGAL_ACCESS;
    }
    return SUCCESS;
}



RETURN_CODE resolveConstantPoolClassRef(CONSTANT_POOL cp, UINT32 cpIndex, LOADED_CLASS_DEF pCurrentClass, COMMON_CLASS_DEF *ppClassDef) 
{
    CLASS_INSTANCE pClassInstance;
    COMMON_CLASS_DEF pClassDef;
    RETURN_CODE ret;
    NamePackage classKey = getConstantPoolNamePackage(cp, cpIndex); /* ANSI union assignment */
    ret = resolveClassRef(pCurrentClass, classKey, &pClassInstance);
    if(ret != SUCCESS) {
        return ret;
    }
    pClassDef = pClassInstance->pRepresentativeClassDef;
#if RUN_TIME_CONSTANT_POOL_RESOLUTION
    enterCritical();
    indicateConstantPoolClassIsResolved(cp, cpIndex);
    setConstantPoolClassDef(cp, cpIndex, pClassDef);
    exitCritical();
#endif

    


    *ppClassDef = pClassDef;
    return SUCCESS;
}



/* fieldModify being TRUE means we are attempting to modify the field - putfield and putstatic */

RETURN_CODE resolveConstantPoolFieldRef(CONSTANT_POOL cp, UINT32 cpIndex, LOADED_CLASS_DEF pCurrentClass, 
                                        BOOLEAN isFieldModify, BOOLEAN isStatic, FIELD_DEF *ppField)
{
    RETURN_CODE ret;
    NameType nameType;
    FIELD_DEF pField;
    LOADED_CLASS_DEF pTargetClassDef;
    CONSTANT_POOL_FIELD cpField = getConstantPoolField(cp, cpIndex);
    
    if(constantPoolClassIsResolved(cp, cpField->classIndex)) {
        pTargetClassDef = (LOADED_CLASS_DEF) getConstantPoolClassDef(cp, cpField->classIndex);
    }
    else if((ret = resolveConstantPoolClassRef(cp, cpField->classIndex, pCurrentClass, (COMMON_CLASS_DEF *) &pTargetClassDef)) != SUCCESS) {
        return ret;
    }
    nameType = getConstantPoolNameType(cp, cpField->nameTypeIndex); /* ANSI union assignment */

    LOG_FIELD_REF("Resolving field", nameType);
    
    ret = isStatic ? 
        lookupStaticField(pTargetClassDef, nameType, &pField) : 
        lookupInstanceField(pTargetClassDef, nameType, &pField);
    if(ret == ERROR_CODE_NO_SUCH_FIELD) {
        /* try finding it in the wrong list */
        ret = isStatic ? 
            lookupInstanceField(pTargetClassDef, nameType, &pField) : 
            lookupStaticField(pTargetClassDef, nameType, &pField);
        if(ret == SUCCESS) {
            return ERROR_CODE_INCOMPATIBLE_CLASS_CHANGE;
        }
        return ERROR_CODE_NO_SUCH_FIELD;
    }
    if(!isClassMemberAccessible((COMMON_CLASS_DEF) pCurrentClass, ((COMMON_CLASS_DEF) (pField->pOwningClass)), pField->accessFlags)) {
        return ERROR_CODE_ILLEGAL_ACCESS;
    }
    if(isFieldModify && isFinalField(pField) && pField->pOwningClass != pCurrentClass) {
        return ERROR_CODE_ILLEGAL_ACCESS;
    }
#if RUN_TIME_CONSTANT_POOL_RESOLUTION
    enterCritical();
    indicateConstantPoolFieldIsResolved(cp, cpIndex);
    setConstantPoolFieldDef(cp, cpIndex, pField);
    exitCritical();
#endif
    *ppField = pField;
    return SUCCESS;
}


RETURN_CODE resolveConstantPoolMethodRef(CONSTANT_POOL cp, UINT32 cpIndex, LOADED_CLASS_DEF pCurrentClass, 
                                         BOOLEAN isStatic, BOOLEAN isVirtual, METHOD_DEF *ppMethod)
{
    LOADED_CLASS_DEF pTargetClassDef;
    RETURN_CODE ret;
    NameType nameType;
    CONSTANT_POOL_METHOD cpMethod = getConstantPoolMethod(cp, cpIndex);
    
    

    if(constantPoolClassIsResolved(cp, cpMethod->classIndex)) {
        pTargetClassDef = (LOADED_CLASS_DEF) getConstantPoolClassDef(cp, cpMethod->classIndex);
    }
    else {
        if((ret = resolveConstantPoolClassRef(cp, cpMethod->classIndex, pCurrentClass, (COMMON_CLASS_DEF *) &pTargetClassDef)) != SUCCESS) {
            return ret;
        }
    }
    


    if(isInterface(pTargetClassDef)) {
        return ERROR_CODE_INCOMPATIBLE_CLASS_CHANGE;
    }
    nameType = getConstantPoolNameType(cp, cpMethod->nameTypeIndex); /* ANSI union assignment */
    
    LOG_METHOD_REF("Resolving method", nameType);
    
    

    ret = isStatic ? 
        lookupStaticMethod(pTargetClassDef, nameType, ppMethod) : 
        lookupMethod(pTargetClassDef, nameType, isVirtual, ppMethod);
    if(ret != SUCCESS) {
        /* try finding the method in the wrong places */
        if(isVirtual) {
            /* try the invokespecial methods (private, init) */
            ret = lookupMethod(pTargetClassDef, nameType, FALSE, ppMethod);
        }
        if(ret != SUCCESS) {
            /* try switching lists */
            ret = isStatic ? 
                lookupMethod(pTargetClassDef, nameType, FALSE, ppMethod) : 
                lookupStaticMethod(pTargetClassDef, nameType, ppMethod);
        }
        if(ret == SUCCESS) {
            return ERROR_CODE_INCOMPATIBLE_CLASS_CHANGE;
        }
        if(isStatic) {
            return ERROR_CODE_NO_SUCH_METHOD;
        }
        else {
            /* this error code may seem strange but it conforms to the spec */
            return ERROR_CODE_ABSTRACT_METHOD;
        }
    }
    if(!isClassMemberAccessible((COMMON_CLASS_DEF) pCurrentClass, (COMMON_CLASS_DEF) ((*ppMethod)->base.pOwningClass), (*ppMethod)->classBase.accessFlags)) {
        return ERROR_CODE_ILLEGAL_ACCESS;
    }
    if(!isStatic && isAbstractMethod(*ppMethod) && !isAbstractClass(pTargetClassDef)) {
        return ERROR_CODE_ABSTRACT_METHOD;
    }

#if RUN_TIME_CONSTANT_POOL_RESOLUTION
    enterCritical();
    indicateConstantPoolMethodIsResolved(cp, cpIndex);
    setConstantPoolMethodDef(cp, cpIndex, *ppMethod);
    exitCritical();
#endif
    return SUCCESS;
}



/* DON'T forget that if this returns no such method, to try to find the method in java.lang.Object,
 * and then perform an invoke virtual (ie see if the method was overridden)
 *
 * This will cover the folling scenario:
 * -Interface I specifies a method, say toString(), which is the same as a method in Object
 * -a method call calls the method through an interface reference, generating an interface method reference
 * -I is recompiled with toString() removed
 * -the interface method reference should still work according to the spec
 */

/* note that we can obtain the index of the method in the interface by pointer substraction in the owning class of ppMethod */

RETURN_CODE resolveConstantPoolInterfaceMethodRef(CONSTANT_POOL cp, UINT32 cpIndex, LOADED_CLASS_DEF pCurrentClass, INTERFACE_METHOD_DEF *ppMethod)
{    
    RETURN_CODE ret;
    NameType nameType;
    LOADED_CLASS_DEF pTargetInterfaceDef;
    CONSTANT_POOL_INTERFACE_METHOD cpInterfaceMethod = getConstantPoolInterfaceMethod(cp, cpIndex);
    
    if(constantPoolClassIsResolved(cp, cpInterfaceMethod->classIndex)) {
        pTargetInterfaceDef = (LOADED_CLASS_DEF) getConstantPoolClassDef(cp, cpInterfaceMethod->classIndex);
    }
    else if((ret = resolveConstantPoolClassRef(cp, cpInterfaceMethod->classIndex, pCurrentClass, (COMMON_CLASS_DEF *) &pTargetInterfaceDef)) != SUCCESS) {
        return ret;
    }
    if(!isInterface(pTargetInterfaceDef)) {
        return ERROR_CODE_INCOMPATIBLE_CLASS_CHANGE;
    }
    nameType = getConstantPoolNameType(cp, cpInterfaceMethod->nameTypeIndex); /* ANSI union assignment */
    
    LOG_METHOD_REF("Resolving interface method", nameType);
    
    ret = lookupMethod(pTargetInterfaceDef, nameType, TRUE, (METHOD_DEF *) ppMethod);
    if(ret != SUCCESS) {
        /* no point in searching for this method elsewhere, 
         * the only possible static method is clinit (so no point in looking at static methods)
         * and all of interface methods but clinit are virtual (so no point in checking invokespecial methods)
         */
        return ERROR_CODE_NO_SUCH_METHOD;
    }
    /* no point in checking method accessibility because the method must be public when loaded */
#if RUN_TIME_CONSTANT_POOL_RESOLUTION
    enterCritical();
    indicateConstantPoolInterfaceMethodIsResolved(cp, cpIndex);
    setConstantPoolInterfaceMethodDef(cp, cpIndex, *ppMethod);
    exitCritical();
#endif
    return SUCCESS;
}


