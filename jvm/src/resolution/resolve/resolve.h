#ifndef RESOLVE_H
#define RESOLVE_H

#include "elements/base.h"
#include "interpret/returnCodes.h"
//#include "interpret/globals.h"
#include "resolution/constantPool.h"


RETURN_CODE obtainClassRef(NamePackage classKey, CLASS_INSTANCE *ppClassInstance);
RETURN_CODE resolveClassRef(LOADED_CLASS_DEF pCurrentClass, NamePackage classKey, CLASS_INSTANCE *ppClassInstance);
RETURN_CODE resolveConstantPoolClassRef(CONSTANT_POOL cp, UINT32 cpIndex, LOADED_CLASS_DEF pCurrentClass, COMMON_CLASS_DEF *ppClassDef);
RETURN_CODE resolveConstantPoolMethodRef(CONSTANT_POOL cp, UINT32 cpIndex, LOADED_CLASS_DEF pCurrentClass, BOOLEAN isStatic, BOOLEAN isVirtual, METHOD_DEF *ppMethod);
RETURN_CODE resolveConstantPoolFieldRef(CONSTANT_POOL cp, UINT32 cpIndex, LOADED_CLASS_DEF pCurrentClass, BOOLEAN isFieldModify, BOOLEAN isStatic, FIELD_DEF *ppField);
RETURN_CODE resolveConstantPoolInterfaceMethodRef(CONSTANT_POOL cp, UINT32 cpIndex, LOADED_CLASS_DEF pCurrentClass, INTERFACE_METHOD_DEF *ppMethod);



#define isClassAccessible(pCurrentClassDef, pTargetClassDef)                            \
    (isPublicClass(pTargetClassDef) ||                                                  \
    getPackageKey(pCurrentClassDef) == getPackageKey(pTargetClassDef))

#define isClassMemberAccessible(currentClass, memberClass, memberAccessFlags)           \
    (                                                                                   \
        (currentClass) == (memberClass) ||                                              \
        (ACC_PUBLIC & (memberAccessFlags)) ||                                           \
        (                                                                               \
            !(ACC_PRIVATE & (memberAccessFlags)) &&                                     \
            (                                                                           \
                getPackageKey(currentClass) == getPackageKey(memberClass) ||            \
                (                                                                       \
                    (ACC_PROTECTED & (memberAccessFlags)) &&                            \
                        isParentClass(memberClass, currentClass)                        \
                )                                                                       \
            )                                                                           \
        )                                                                               \
    )


#endif