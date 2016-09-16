#ifndef BYTECODEFILTER_H
#define BYTECODEFILTER_H

#include "elements/base.h"

/* turning this on allows you to replace java opcodes with implementation specific instructions */

#define BYTECODE_FILTERING YES

void filterClass(LOADED_CLASS_DEF pClassDef);

#endif