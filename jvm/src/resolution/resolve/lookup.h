#ifndef LOOKUP_H
#define LOOKUP_H

#include "elements/base.h"
#include "interpret/returnCodes.h"

/* fields */

RETURN_CODE lookupInstanceField(LOADED_CLASS_DEF pClassDef, NameType fieldKey, FIELD_DEF *ppField);
RETURN_CODE lookupStaticField(LOADED_CLASS_DEF pClassDef, NameType fieldKey, FIELD_DEF *ppField);

/* methods */

RETURN_CODE lookupMethod(LOADED_CLASS_DEF pClassDef, NameType methodKey, BOOLEAN isVirtual, METHOD_DEF *ppMethod);
RETURN_CODE lookupStaticMethod(LOADED_CLASS_DEF pClassDef, NameType methodKey, METHOD_DEF *ppMethod);


#endif