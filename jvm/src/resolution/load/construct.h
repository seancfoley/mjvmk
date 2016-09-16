#ifndef CONSTRUCT_H
#define CONSTRUCT_H

#include "elements/base.h"
#include "interpret/returnCodes.h"
#include "load.h"

RETURN_CODE constructClass(LOAD_DATA pLoadData, CONSTANT_POOL pConstantPool, 
                                  CLASS_INSTANCE *ppClassInstance, LOADED_CLASS_DEF *ppClassDef);


#endif
