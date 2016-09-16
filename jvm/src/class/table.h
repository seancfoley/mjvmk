#ifndef TABLE_H
#define TABLE_H

#include "elements/base.h"
#include "interpret/returnCodes.h"


/* method access */


BOOLEAN classIsInTableGeneric(CLASS_ENTRY classTableBase, NamePackageKey key, UINT16 classTableSize, CLASS_ENTRY *ppClassEntry);
RETURN_CODE getClassInTableGeneric(CLASS_ENTRY classTableBase, NamePackageKey key, UINT16 classTableSize, CLASS_ENTRY *ppClassEntry);
COMMON_CLASS_DEF getClassDefInTable(NamePackageKey key);

#define getClassInTable(key, ppClassEntry) (getClassInTableGeneric(getClassTableBase(getClassTable()), key, getClassTableSize(getClassTable()), ppClassEntry))
#define classIsInTable(key, ppClassEntry) classIsInTableGeneric(getClassTableBase(getClassTable()), key, getClassTableSize(getClassTable()), ppClassEntry)
#define classIsLoaded(key, ppClassEntry) (classIsInTable(key, ppClassEntry) && classInTableIsLoaded(*(ppClassEntry)))

RETURN_CODE initializeClassTable();

#endif

