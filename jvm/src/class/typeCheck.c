#include "interpret/javaGlobals.h"
#include "typeCheck.h"

/*
 * determines if implementsClassDef is a class that implements interfaceClassDef
 * or if implementsClassDef is a child interface of interfaceClassDef
 *
 * In other words, determines is interfaceClassDef is a superinterface of implementsClassDef
 */
BOOLEAN isSuperInterface(COMMON_CLASS_DEF pInterfaceClassDef, COMMON_CLASS_DEF pImplementsClassDef)
{
    if(isArrayClass(pImplementsClassDef) || !isInterface(pInterfaceClassDef)) {
        return FALSE;
    }
    else if(getInterfaceList(getLoadedClassDef(pImplementsClassDef)) != NULL) {
        UINT16 i;
        UINT16 length = getInterfaceListLength(getLoadedClassDef(pImplementsClassDef));
        
        for(i=0; i<length; i++) {
            if(getLoadedInterfaceDef(pInterfaceClassDef) == getSuperInterface(getLoadedClassDef(pImplementsClassDef), i)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* 
 * Determines if the class parentClassDef is a parent of childClassDef.
 * Note that the parent of all array classes is Object.
 */
BOOLEAN isParentClass(COMMON_CLASS_DEF pParentClassDef, COMMON_CLASS_DEF pChildClassDef)
{
    do {
        pChildClassDef = &(getParent(pChildClassDef)->header);
        if(pParentClassDef == pChildClassDef) {
            return TRUE;
        }
     } while(pChildClassDef != NULL);
    return FALSE;
}



/* see instanceof or checkcast in VM spec for the algorithm description */
BOOLEAN isInstanceOf(COMMON_CLASS_DEF pTargetClassDef, COMMON_CLASS_DEF pSourceClassDef)
{
    do {
        /* nothing to do if they are the same class or interface */
        if(pTargetClassDef == pSourceClassDef) {
            return TRUE;
        }

        if(isLoadedClassOrInterface(pSourceClassDef)) {
            if(isInterface(pSourceClassDef)) {
                /* if source is an interface, target must be a superinterface or java.lang.Object */
                return isSuperInterface(pTargetClassDef, pSourceClassDef)
                    || pTargetClassDef == (COMMON_CLASS_DEF) pJavaLangObjectDef;
            }
            if(isInterface(pTargetClassDef)) {
                /* if target is an interface, then source must be an interface or object that implements target */
                return isSuperInterface(pTargetClassDef, pSourceClassDef);
            }
            return isParentClass(pTargetClassDef, pSourceClassDef);
        }

        /* source is an array class */
        
        if(isLoadedClassOrInterface(pTargetClassDef)) {
              /* the target must be either Object, Cloneable or Serializable.
               * Cloneable and Serializable are not in CLDC.
               */
            return pTargetClassDef == (COMMON_CLASS_DEF) pJavaLangObjectDef;
        }

        /* target is also an array class */

        if(isPrimitiveArrayClass(pSourceClassDef) || isPrimitiveArrayClass(pTargetClassDef)) {
            /* we've already checked if the classes are the same */
            return FALSE;
        }
        
        /* two non-primitive array classes */
        pTargetClassDef = getElementClass(getArrayClassDef(pTargetClassDef));
        pSourceClassDef = getElementClass(getArrayClassDef(pSourceClassDef));

    } while(TRUE);
}
