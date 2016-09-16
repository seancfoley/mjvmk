#ifndef PREPARE_H
#define PREPARE_H

#include "elements/base.h"
#include "interpret/returnCodes.h"

#if PREPARE_WHILE_LOADING
void checkForClassInitializer(CLASS_INSTANCE pClassInstance, COMMON_CLASS_DEF pClassDef);
#else
RETURN_CODE iterativePrepareClass(CLASS_ENTRY pClassEntry);
#endif

void initializeClassInstance(CLASS_INSTANCE pClassInstance, COMMON_CLASS_DEF pClassDef, MONITOR pMonitor, 
                                    BOOLEAN hasFields);

#endif

