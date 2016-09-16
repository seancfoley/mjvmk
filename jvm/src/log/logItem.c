
#include "operate/log.h"
#include "string/stringPool.h"
#include "string/stringKeys.h"
#include "logItem.h"

#if LOGGING



char *getIndentation(int indentation)
{
    static char indent[8] = "       ";
    static char *indents[8] = {indent + 7, indent + 6, indent + 5, indent + 4,
        indent + 3, indent + 2, indent + 1, indent};

    if(indentation<=7) {
        return indents[indentation];
    }
    return indents[0];
}


void logNameType(int indentation, NameType key, BOOLEAN isMethod)
{
    UINT16 nameLength;
    char *name;
    UINT16 typeLength;
    char *type;
    char formatString[25];

    name = getUTF8StringByKey(key.nt.nameKey, &nameLength);
    type = isMethod ? getMethodTypeStringByKey(key.nt.typeKey, &typeLength) : getFieldTypeStringByKey(key.nt.typeKey, &typeLength);
    stringPrintf((formatString, "%%s%%.%ds %%.%ds", nameLength, typeLength));
    logPrintf(formatString, getIndentation(indentation), name, type);
}

void logClassName(int indentation, NamePackage key)
{
    char *pkg; 
    char *name;
    UINT16 pkgLength;
    UINT16 nameLength;
    char formatString[25];

    pkg = getPackageNameStringByKey(key.np.packageKey, &pkgLength);
    name = getUTF8StringByKey(key.np.nameKey, &nameLength);
    stringPrintf((formatString, "%%s%%s%%.%ds%%s%%.%ds", pkgLength, nameLength));
    logPrintf(formatString, getIndentation(indentation), getArrayDepthString(key.np.packageKey), pkg, pkgLength ? "/" : "", name);
}

void logObjectRef(char *prefix, OBJECT_INSTANCE pObject)
{
    enterLoggingMonitor();
    logPrintf("%s: instance %x of class ", prefix, pObject);
    logClassName(0, pObject->header.pClassDef->key);
    logNewLine();
    exitLoggingMonitor();
    return;
}

void logClassRef(char *prefix, NamePackage classKey)
{
    enterLoggingMonitor();
    logPrintf("%s: ", prefix);
    logClassName(0, classKey);
    logNewLine();
    exitLoggingMonitor();
    return;
}

void logMethodRef(char *prefix, NameType nameType)
{
    enterLoggingMonitor();
    logPrintf("%s: ", prefix);
    logNameType(0, nameType, TRUE);
    logNewLine();
    exitLoggingMonitor();
    return;
}

void logFieldRef(char *prefix, NameType nameType)
{
    enterLoggingMonitor();
    logPrintf("%s: ", prefix);
    logNameType(0, nameType, FALSE);
    logNewLine();
    exitLoggingMonitor();
    return;
}

void logMethodCall(char *prefix, METHOD_DEF pMethod, OBJECT_INSTANCE pObject)
{
    enterLoggingMonitor();
    logPrintf("%s: ", prefix);
    logClassName(0, pObject->header.pClassDef->key);
    logChar('.');
    logNameType(0, pMethod->base.nameType, TRUE);
    logNewLine();
    exitLoggingMonitor();
    return;
}

void logMethod(char *prefix, METHOD_DEF pMethod)
{
    enterLoggingMonitor();
    logPrintf("%s: ", prefix);
    logClassName(0, pMethod->base.pOwningClass->header.key);
    logChar('.');
    logNameType(0, pMethod->base.nameType, TRUE);
    logNewLine();
    exitLoggingMonitor();
    return;
}






#endif