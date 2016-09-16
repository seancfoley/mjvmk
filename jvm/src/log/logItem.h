#ifndef LOGITEM_H
#define LOGITEM_H

#include "operate/log.h"
#include "elements/base.h"

#if LOGGING


char *getIndentation(int indentation);
void logNameType(int indentation, NameType key, BOOLEAN isMethod);
void logClassName(int indentation, NamePackage key);
void logObjectRef(char *prefix, OBJECT_INSTANCE pObject);
void logMethod(char *prefix, METHOD_DEF pMethodDef);
void logMethodRef(char *prefix, NameType nameType);
void logFieldRef(char *prefix, NameType nameType);
void logClassRef(char *prefix, NamePackage namePackage);
void logMethodCall(char *prefix, METHOD_DEF pMethod, OBJECT_INSTANCE pObject);

#define LOG_OBJECT(prefix, pObject) logObjectRef(prefix, pObject)
#define LOG_CLASS_REF(prefix, namePackage) logClassRef(prefix, namePackage)
#define LOG_FIELD_REF(prefix, nameType) logFieldRef(prefix, nameType)
#define LOG_METHOD_REF(prefix, nameType) logMethodRef(prefix, nameType)
#define LOG_METHOD(prefix, pMethodDef) logMethod(prefix, pMethodDef)
#define LOG_METHOD_CALL(prefix, pMethodDef, pObject) logMethodCall(prefix, pMethodDef, pObject)
#define LOG_PATH(x) LOG_LINE(x)

#else

#define LOG_OBJECT(prefix, pObject)
#define LOG_CLASS_REF(prefix, namePackage)
#define LOG_FIELD_REF(prefix, nameType)
#define LOG_METHOD_REF(prefix, nameType)
#define LOG_METHOD(prefix, pMethodDef)
#define LOG_METHOD_CALL(prefix, pMethodDef, pObject)
#define LOG_PATH(x)

#endif



#endif