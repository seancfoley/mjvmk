#ifndef LOGCLASS_H
#define LOGCLASS_H

#include "elements/base.h"

#if LOGGING

void logClass(int indentation, LOADED_CLASS_DEF pClassDef);
#define LOG_CLASS(pClassDef) logClass(0, (LOADED_CLASS_DEF) pClassDef)

#else

#define LOG_CLASS(pClassDef)

#endif

#endif