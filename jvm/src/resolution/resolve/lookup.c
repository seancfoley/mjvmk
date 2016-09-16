
#include "lookup.h"


RETURN_CODE lookupInstanceField(LOADED_CLASS_DEF pClassDef, NameType fieldKey, FIELD_DEF *ppField)
{
    FIELD_LIST pFieldList;
    FIELD_DEF pCurrentField;
    UINT16 count;
    FIELD_DEF *ppFields;
    
    do {
        pFieldList = pClassDef->pInstanceFields;
        count = pFieldList ? (ppFields = pFieldList->ppFields, pFieldList->length) : 0;
        
        /* we start at the top, where the current class fields are located, 
         * heading towards the parent class fields 
         */
        while(count != 0) {
            --count;
            pCurrentField = ppFields[count];    
            if (getFieldKey(pCurrentField) == fieldKey.nameTypeKey) { 
                *ppField = pCurrentField;
                return SUCCESS;
            }
        }
        pClassDef = pClassDef->header.pSuperClass;

    } while(pClassDef != NULL);
    *ppField = NULL;
    return ERROR_CODE_NO_SUCH_FIELD;
}


RETURN_CODE lookupStaticField(LOADED_CLASS_DEF pClassDef, NameType fieldKey, FIELD_DEF *ppField)
{
    FIELD_LIST pFieldList;
    FIELD_DEF pCurrentField;
    UINT16 count;
    FIELD_DEF *ppFields;
    INTERFACE_LIST pSuperInterfaceList = pClassDef->loadedHeader.pSuperInterfaceList;
    UINT16 interfaceCount = (pSuperInterfaceList == NULL) ? 0 : pSuperInterfaceList->length;

    do {
        pFieldList = pClassDef->loadedHeader.pStaticFields;
        count = pFieldList ? (ppFields = pFieldList->ppFields, pFieldList->length) : 0;
        
        while(count != 0) {
            --count;
            pCurrentField = ppFields[count];    
            if (getFieldKey(pCurrentField) == fieldKey.nameTypeKey) { 
                *ppField = pCurrentField;
                return SUCCESS;
            }
        }
        if(interfaceCount) {
            interfaceCount--;
            pClassDef = (LOADED_CLASS_DEF) (pSuperInterfaceList->pInterfaceDefs[interfaceCount]);
        }
        else {
            pClassDef = pClassDef->header.pSuperClass;
        }
    } while(pClassDef != NULL);
    *ppField = NULL;
    return ERROR_CODE_NO_SUCH_FIELD;
}

RETURN_CODE lookupSuperInterfaceMethod(LOADED_CLASS_DEF pClassDef, NameType methodKey, METHOD_DEF *ppMethod)
{
    METHOD_LIST pMethodList;
    METHOD_DEF pCurrentMethod;
    METHOD_DEF *ppMethods;
    UINT16 count;
    LOADED_INTERFACE_DEF pInterfaceDef;
    INTERFACE_LIST pSuperInterfaceList = pClassDef->loadedHeader.pSuperInterfaceList;
    UINT16 interfaceCount = (pSuperInterfaceList == NULL) ? 0 : pSuperInterfaceList->length;
    
    while(interfaceCount != 0) {
        interfaceCount--;
        pInterfaceDef = pSuperInterfaceList->pInterfaceDefs[interfaceCount];
        pMethodList = getInstanceMethodList(pInterfaceDef);
        count = pMethodList ? (ppMethods = pMethodList->ppMethods, pMethodList->length) : 0;
        while(count != 0) {
            --count;
            pCurrentMethod = ppMethods[count];
            if (getMethodKey(pCurrentMethod) == methodKey.nameTypeKey) {
                *ppMethod = pCurrentMethod;
                return SUCCESS;
            }
        }
    
    }
    *ppMethod = NULL;
    return ERROR_CODE_NO_SUCH_METHOD;
}

RETURN_CODE lookupMethod(LOADED_CLASS_DEF pClassDef, NameType methodKey, BOOLEAN isVirtual, METHOD_DEF *ppMethod)
{
    METHOD_LIST pMethodList;
    METHOD_DEF pCurrentMethod;
    UINT16 count;
    METHOD_DEF *ppMethods;
    LOADED_CLASS_DEF pCurrentClassDef = pClassDef;
     
    do {
        pMethodList = getInstanceMethodList(pCurrentClassDef);
        count = pMethodList ? (ppMethods = pMethodList->ppMethods, (isVirtual ? pCurrentClassDef->header.virtualMethodCount : pMethodList->length)) : 0;
        
        /* we start at the top, where the current class methods are located, 
         * heading towards the parent class methods
         */
        while(count != 0) {
            --count;
            pCurrentMethod = ppMethods[count];
            if (getMethodKey(pCurrentMethod) == methodKey.nameTypeKey) {
                *ppMethod = pCurrentMethod;
                return SUCCESS;
            }
        }
        pCurrentClassDef = pCurrentClassDef->header.pSuperClass;
    } while(pCurrentClassDef != NULL);

    /* Now see if it is an unimplemented interface method (if pClassDef is a class)
     * or an inherited interface method (if pClassDef is an interface)
     */
    return lookupSuperInterfaceMethod(pClassDef, methodKey, ppMethod);
}

/* this lookup will not find the clinit method because it is always last in the list, 
 * and is not counted in the list length.
 */
RETURN_CODE lookupStaticMethod(LOADED_CLASS_DEF pClassDef, NameType methodKey, METHOD_DEF *ppMethod)
{
    METHOD_LIST pMethodList;
    METHOD_DEF pCurrentMethod;
    UINT16 count;
    METHOD_DEF *ppMethods;
     
    do {
        pMethodList = getStaticMethodList(pClassDef);
        count = pMethodList ? (ppMethods = pMethodList->ppMethods, pMethodList->length) : 0;
        while(count != 0) {
            --count;
            pCurrentMethod = ppMethods[count];
            if (getMethodKey(pCurrentMethod) == methodKey.nameTypeKey) {
                *ppMethod = pCurrentMethod;
                return SUCCESS;
            }
        }
        pClassDef = pClassDef->header.pSuperClass;
    } while(pClassDef != NULL);
    *ppMethod = NULL;
    return ERROR_CODE_NO_SUCH_METHOD;
}

