
#include "string/stringPool.h"
#include "resolution/constantPool.h"
#include "logItem.h"
#include "logClass.h"


#if LOGGING

static void logString(int indentation, STRING_INSTANCE pString)
{
    jint i;
    char c;
    CHAR_ARRAY_INSTANCE pCharArray = (CHAR_ARRAY_INSTANCE) extractObjectFieldObject(pString->pCharArray);


    logPrintf("%s", getIndentation(indentation));
    for(i=0; i< extractPrimitiveFieldInt(pString->length); i++) {
        c = (char) pCharArray->values[i];
        logChar(c);
    }
}

static void logInternedString(int indentation, INTERNED_STRING_INSTANCE pString)
{
    logString(indentation, (STRING_INSTANCE) pString);
}


static void logClassMemberRef(int indentation, NamePackage classKey, NameType key, BOOLEAN isMethod)
{
    logPrintf("%sref: ", getIndentation(indentation));
    logClassName(0, classKey);
    logChar('.');
    logNameType(0, key, isMethod);
}

static void logClassFieldRef(int indentation, NamePackage classKey, NameType key)
{
    logPrintf("%sfield ", getIndentation(indentation));
    logClassMemberRef(0, classKey, key, FALSE);
}

static void logClassMethodRef(int indentation, NamePackage classKey, NameType key)
{
    logPrintf("%smethod ", getIndentation(indentation));
    logClassMemberRef(0, classKey, key, TRUE);
}

static void logClassInterfaceMethodRef(int indentation, NamePackage classKey, NameType key)
{
    logPrintf("%sinterface ", getIndentation(indentation));
    logClassMethodRef(0, classKey, key);
}

static void logUnresolvedConstantPool(int indentation, CONSTANT_POOL pConstantPool)
{
    UINT32 i;
    
    logPrintf("%sConstant pool:\n", getIndentation(indentation));
    for(i=1; i<getConstantPoolLength(pConstantPool); i++) {
        logPrintf("%sEntry %d: ", getIndentation(indentation + 1), i);
        switch(getConstantPoolUnmaskedTag(pConstantPool, i)) {
            case CONSTANT_Utf8:
                /* these are jettisoned from the constant pool and so we should not arrive here */
                logPrintf("UTF8 string\n"); 
                break;
            case CONSTANT_Integer:
                logPrintf("integer: %d\n", getConstantPoolInteger(pConstantPool, i));
                break;
            case CONSTANT_Long: {
                jlong val = getConstantPoolLong(pConstantPool, i);
                logPrintf("long: %d\n", (val.high << 32) + val.low);
                break;
            }
#if IMPLEMENTS_FLOAT
            case CONSTANT_Float:
                logPrintf("float: %g\n", getConstantPoolFloat(pConstantPool, i)); 
                break;
            case CONSTANT_Double:
                logPrintf("double: %g\n", getConstantPoolDouble(pConstantPool, i));
                break;
#endif
            case CONSTANT_Class: {
                if(!constantPoolClassIsResolved(pConstantPool, i)) {
                    logPrintf("class: ");
                    logClassName(0, getConstantPoolNamePackage(pConstantPool, i));
                }
                logNewLine();
                break;
            }
            case CONSTANT_String:
                logPrintf("interned string: ");
                logInternedString(0, getConstantPoolInternedString(pConstantPool, i));
                logNewLine();
                break;
            case CONSTANT_InterfaceMethodref: {
                if(!constantPoolInterfaceMethodIsResolved(pConstantPool, i)) {
                    CONSTANT_POOL_INTERFACE_METHOD pI = getConstantPoolInterfaceMethod(pConstantPool, i);
                    logClassInterfaceMethodRef(0, getConstantPoolNamePackage(pConstantPool, pI->classIndex), getConstantPoolNameType(pConstantPool, pI->nameTypeIndex));
                }
                logNewLine();
                break;
            }
            case CONSTANT_Methodref: {
                if(!constantPoolMethodIsResolved(pConstantPool, i)) {
                    CONSTANT_POOL_METHOD pM = getConstantPoolMethod(pConstantPool, i);
                    logClassMethodRef(0, getConstantPoolNamePackage(pConstantPool, pM->classIndex), getConstantPoolNameType(pConstantPool, pM->nameTypeIndex));
                }
                logNewLine();
                break;
            }
            case CONSTANT_Fieldref: {
                if(!constantPoolFieldIsResolved(pConstantPool, i)) {
                    CONSTANT_POOL_FIELD pField = getConstantPoolField(pConstantPool, i);
                    logClassFieldRef(0, getConstantPoolNamePackage(pConstantPool, pField->classIndex), getConstantPoolNameType(pConstantPool, pField->nameTypeIndex));
                }
                logNewLine();
                break;
            }
            case CONSTANT_FieldNameAndType:
            case CONSTANT_MethodNameAndType: {
                logPrintf("name type: ");
                logNameType(0, getConstantPoolNameType(pConstantPool, i), (BOOLEAN) (getConstantPoolUnmaskedTag(pConstantPool, i) == CONSTANT_FieldNameAndType));
                logNewLine();
                break;
            }
            default:
                logPrintf("Unknown or empty entry\n");
                break;
        } /* end switch */
    } /* end for */
}



void logAccessFlags(int indentation, UINT16 accessFlags, BOOLEAN isClass)
{
    logPrintf("%saccess flags:", getIndentation(indentation));
    if(accessFlags & ACC_PUBLIC) logPrintf(" PUBLIC");
    if(accessFlags & ACC_FINAL) logPrintf(" FINAL");
    if(isClass && (accessFlags & ACC_SUPER)) logPrintf(" SUPER");
    if(isClass && (accessFlags & ACC_INTERFACE)) logPrintf(" INTERFACE");
    if(!isClass && (accessFlags & ACC_FILTERED)) logPrintf(" FILTERED");
    if(accessFlags & ACC_ABSTRACT) logPrintf(" ABSTRACT");
    if(accessFlags & ACC_PRIMITIVE) logPrintf(" PRIMITIVE");
    if(accessFlags & ACC_INIT) logPrintf(" INIT");
    if(accessFlags & ACC_PRIVATE) logPrintf(" PRIVATE");
    if(accessFlags & ACC_PROTECTED) logPrintf(" PROTECTED");
    if(accessFlags & ACC_STATIC) logPrintf(" STATIC");
    if(accessFlags & ACC_VOLATILE) logPrintf(" VOLATILE");
    if(accessFlags & ACC_TRANSIENT) logPrintf(" TRANSIENT");
    if(!isClass && (accessFlags & ACC_SYNCHRONIZED)) logPrintf(" SYNCHRONIZED");
    if(accessFlags & ACC_NATIVE) logPrintf(" NATIVE");
    if(accessFlags & ACC_STRICT) logPrintf(" STRICT");
}

void logThrowableHandler(int indentation, THROWABLE_HANDLER pHandler)
{
    logPrintf("%sstart: %d end: %d handler: %d ", getIndentation(indentation), 
            pHandler->startPCIndex, pHandler->endPCIndex, pHandler->handlerPCIndex);
    if(pHandler->catchKey.namePackageKey == 0) {
        logPrintf("Catches everything");
    }
    else {
        logClassName(0, pHandler->catchKey);
    }
}

void logThrowableList(int indentation, THROWABLE_HANDLER_LIST pList) 
{
    UINT16 i;
    
    logPrintf("%sHandlers:\n", getIndentation(indentation));
    for(i=0; i < pList->length; i++) {
        logThrowableHandler(indentation + 1, pList->ppHandlers[i]);
        logNewLine();
    }
    logNewLine();
}

void logBytecodeStream(int indentation, METHOD_DEF pMethod)
{
    UINT16 i;

    logPrintf("%sbytecode: \n", getIndentation(indentation), pMethod->byteCodeCount);
    logPrintf("%s", getIndentation(indentation + 1));
    for(i=0; i < pMethod->byteCodeCount; i++) {
        logPrintf("%02x ", pMethod->pCode[i]);
    }
    logNewLine();
}

void logClassMethod(int indentation, METHOD_DEF pMethod, UINT16 index)
{
    UINT16 accessFlags;

    logPrintf("%sMethod %u:\n", getIndentation(indentation), index);
    logClassMethodRef(indentation + 1, pMethod->base.pOwningClass->header.key, pMethod->base.nameType);
    logNewLine();
    logPrintf("%stable index: %d\n", getIndentation(indentation + 1), pMethod->base.methodIndex);
    logPrintf("%sparameter word count: %d\n", getIndentation(indentation + 1), pMethod->base.parameterVarCount);
    if(isInterface(&pMethod->base.pOwningClass->header)) {
        return;
    }

    accessFlags = pMethod->classBase.accessFlags;
    logAccessFlags(indentation + 1, accessFlags, FALSE);
    logNewLine();
    if(accessFlags & ACC_ABSTRACT) {
        return;
    }
    if(accessFlags & ACC_NATIVE) {
        /* get the native method name as if we were linking now */
        return;
    }
    
    logPrintf("%slocals word count: %d\n", getIndentation(indentation + 1), pMethod->localVarCount);
    logPrintf("%slocals and parameters word count: %d\n", getIndentation(indentation + 1), pMethod->varCount);
    logPrintf("%sinvoke word count: %d\n", getIndentation(indentation + 1), pMethod->invokeCount);
    logPrintf("%sbytecode count: %d\n", getIndentation(indentation + 1), pMethod->byteCodeCount);
    logBytecodeStream(indentation + 1, pMethod);
    if(pMethod->throwableHandlers) {
        logThrowableList(indentation + 1, pMethod->throwableHandlers);
    }
}

void logMethodList(int indentation, METHOD_LIST pList, UINT16 count) 
{
    UINT16 i;
    
    logPrintf("%sMethods:\n", getIndentation(indentation));
    for(i=0; i<count; i++) {
        logClassMethod(indentation + 1, pList->ppMethods[i], i);
        logNewLine();
    }
    logNewLine();
}


void logClassField(int indentation, FIELD_DEF pField, UINT16 index)
{
    logPrintf("%sField %u:\n", getIndentation(indentation), index);
    logClassFieldRef(indentation + 1, pField->pOwningClass->header.key, pField->key);
    logNewLine();
    logPrintf("%stable index: %d\n", getIndentation(indentation + 1), pField->fieldIndex);
    logAccessFlags(indentation + 1, pField->accessFlags, FALSE);
    if(pField->accessFlags & ACC_INIT) {
        CONSTANT_FIELD_DEF pConstantField = (CONSTANT_FIELD_DEF) pField;

        logPrintf("%sConstant value 1: %x\n", getIndentation(indentation + 1), &pConstantField->value);
        logPrintf("%sConstant value 2: %x\n", getIndentation(indentation + 1), &pConstantField->value2);
        
    }
}

void logFieldList(int indentation, FIELD_LIST pList) 
{
    UINT16 i;

    if(pList == NULL) {
        return;
    }
    logPrintf("%sFields:\n", getIndentation(indentation));
    for(i=0; i<pList->length; i++) {
        logClassField(indentation + 1, pList->ppFields[i], i);
        logNewLine();
    }
    logNewLine();
}

void logInterfaceList(int indentation, INTERFACE_LIST pList) 
{
    UINT16 i;
    
    if(pList == NULL) {
        return;
    }
    logPrintf("%sSuper interfaces:\n", getIndentation(indentation));
    for(i=0; i<pList->length; i++) {
        logClassName(indentation + 1, pList->pInterfaceDefs[i]->header.key);
        logNewLine();
    }
    logNewLine();
}

void logInterfaceMethods(int indentation, METHOD_DEF *ppMethods, METHOD_LIST pInterfaceMethodList)
{
    UINT16 i;
    
    for(i=0; i<pInterfaceMethodList->length; i++) {
        if(ppMethods[i] == NULL) {
            logPrintf("%sMethod %u implementation not found\n", getIndentation(indentation), i);
            continue;
        }
        logClassMethod(indentation, ppMethods[i], i);
        logNewLine();
    }
}

void logInterfaceImplementationTable(int indentation, LOADED_CLASS_DEF pClassDef)
{
    INTERFACE_LIST pList = pClassDef->loadedHeader.pSuperInterfaceList;
    METHOD_DEF **pppMethods; 
    UINT16 i;

    if(pList == NULL) {
        return;
    }
    pppMethods = pList->pppInterfaceImplementationTable;
    if(pppMethods == NULL) {
        return;
    }
    for(i=0; i<pList->length; i++) {
        logPrintf("%sImplementation of interface ", getIndentation(indentation));
        logClassName(0, pList->pInterfaceDefs[i]->header.key);
        logNewLine();
        logInterfaceMethods(indentation + 1, pppMethods[i], getInstanceMethodList(pList->pInterfaceDefs[i]));
    }
    logNewLine();
}

void logClass(int indentation, LOADED_CLASS_DEF pClassDef)
{
    BOOLEAN isInterface = isInterface(pClassDef) != 0;
    BOOLEAN isArrayClass = isArrayClass(pClassDef) != 0;
    
    enterLoggingMonitor();
    
    logClassName(indentation, pClassDef->header.key);
    logNewLine();
    logPrintf("%sclass table index: %u\n", getIndentation(indentation + 1), pClassDef->header.tableIndex);
    logAccessFlags(indentation + 1, pClassDef->header.accessFlags, TRUE);
    logNewLine();
    if(!isArrayClass) {
        logUnresolvedConstantPool(indentation + 1, pClassDef->loadedHeader.pConstantPool);       
    }
    if(pClassDef->header.pSuperClass != NULL) {
        logPrintf("%sSuper class: ", getIndentation(indentation + 1));
        logClassName(0, pClassDef->header.pSuperClass->header.key); 
        logNewLine();
    }
    if(!isArrayClass) {
        logInterfaceList(indentation + 1, pClassDef->loadedHeader.pSuperInterfaceList);
    }
    logPrintf("%svirtual method count: %u\n", getIndentation(indentation + 1), 
        pClassDef->header.virtualMethodCount);
    if(getInstanceMethodList(pClassDef)) {
        logMethodList(indentation + 1, getInstanceMethodList(pClassDef), (UINT16) (isArrayClass ? pClassDef->header.virtualMethodCount : getInstanceMethodList(pClassDef)->length));
    }
    if(!isArrayClass) {
        logFieldList(indentation + 1, pClassDef->loadedHeader.pStaticFields);
        if(!isInterface) {
            if(getStaticMethodList(pClassDef)) {
                logMethodList(indentation + 1, getStaticMethodList(pClassDef), getStaticMethodList(pClassDef)->length);
            }
            if(pClassDef->header.accessFlags & ACC_INIT) {
                logClassMethod(indentation + 1, getClinitMethod(pClassDef), 0);
                logNewLine();
            }
            logFieldList(indentation + 1, pClassDef->pInstanceFields);
            logPrintf("%sInstance size: %u\n\n", getIndentation(indentation + 1), pClassDef->instanceSize);
            logInterfaceImplementationTable(indentation + 1, pClassDef);
            logNewLine();
        }
    }
    exitLoggingMonitor();
}

#endif







