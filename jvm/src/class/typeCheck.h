#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "elements/base.h"

/*
Notes

The checkcast instruction is very similar to the instanceof instruction. 
The differences:
- treatment of null (instanceof is false, checkcast passes)
- behavior when its test fails (checkcast throws an exception, instanceof pushes a result code) 
- effect on the operand stack
*/

/* 
 * Do not pass a NULL instance to the following macro.  In fact, neither parameter may be NULL.
 * Note that the handling of NULL differs between checkcast and instanceof, both of which use this function!
 */
#define isObjectInstanceOf(targetClassDef, pInstance) isInstanceOf((COMMON_CLASS_DEF) targetClassDef, ((COMMON_INSTANCE) pInstance)->pClassDef)

BOOLEAN isInstanceOf(COMMON_CLASS_DEF targetClassDef, COMMON_CLASS_DEF sourceClassDef);
BOOLEAN isParentClass(COMMON_CLASS_DEF parentClassDef, COMMON_CLASS_DEF childClassDef);
BOOLEAN isSuperInterface(COMMON_CLASS_DEF interfaceClassDef, COMMON_CLASS_DEF implementsClassDef);

#endif
