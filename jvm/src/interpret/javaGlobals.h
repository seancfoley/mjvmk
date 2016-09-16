#ifndef JAVA_GLOBALS_H
#define JAVA_GLOBALS_H


#include "elements/base.h"


typedef struct hashTableStruct hashTableStruct, *HASH_TABLE, *STRING_POOL;
typedef struct frameStruct frameStruct, *FRAME;


/* The registers related to a java thread's stack frame -
 * all these registers change with each new method invocation and thread swap. 
 */
typedef struct frameRegisterStruct {
    JSTACK_FIELD sp;
    FRAME fp;
    INSTRUCTION pc;
    JSTACK_FIELD lp;
#if USE_CLASS_REGISTERS
    CONSTANT_POOL pCurrentConstantPool;
    LOADED_CLASS_DEF pCurrentClassDef;
    METHOD_DEF pCurrentMethodDef;
#endif
} frameRegisterStruct, *FRAME_REGISTERS;


/* registers that do not change with each method invocation, but do change with thread swaps */
typedef struct globalRegisterStruct {
    CLASS_TABLE pClassTable;
    STRING_POOL pUnicodeStringPool;
    STRING_POOL pUtf8StringPool;
} globalRegisterStruct, *GLOBAL_REGISTERS;


extern globalRegisterStruct globalRegisters;


#define getClassTable()                 (globalRegisters.pClassTable)
#define getUnicodeStringPool()          (globalRegisters.pUnicodeStringPool)
#define getUTF8StringPool()             (globalRegisters.pUtf8StringPool)


#define setClassTable(pTable)           (globalRegisters.pClassTable = (pTable))
#define setUnicodeStringPool(pPool)     (globalRegisters.pUnicodeStringPool = (pPool))
#define setUTF8StringPool(pPool)        (globalRegisters.pUtf8StringPool = (pPool))

#define getClassInstanceFromClassEntry(pEntry)  ((pEntry)->u.pClassInstance)
#define getClassInTableByIndex(index)           getClassInstanceFromClassEntry(getEntryPtrInTableByIndex(getClassTable(), index))
#define getClassInstanceFromClassDef(pClassDef) getClassInTableByIndex(getClassTableIndexFromClass(pClassDef))
#define getClassEntryFromClassDef(pClassDef)    getEntryPtrInTableByIndex(getClassTable(), getClassTableIndexFromClass(pClassDef))



extern CLASS_INSTANCE pJavaLangObject;
extern CLASS_INSTANCE pJavaLangClass;
extern CLASS_INSTANCE pJavaLangString;
extern CLASS_INSTANCE pJavaLangThread;
extern CLASS_INSTANCE pJavaLangStringArrayClass;
extern CLASS_INSTANCE pCharArrayClass;
extern CLASS_INSTANCE pIntegerArrayClass;
extern CLASS_INSTANCE pShortArrayClass;
extern CLASS_INSTANCE pComMjvmkStackTrace;

extern LOADED_CLASS_DEF pJavaLangObjectDef;
extern LOADED_CLASS_DEF pJavaLangStringDef;
extern LOADED_CLASS_DEF pJavaLangClassDef;
extern LOADED_CLASS_DEF pJavaLangThreadDef;
extern ARRAY_CLASS_DEF pCharArrayClassDef;

extern OBJECT_INSTANCE pJavaLangOutOfMemoryErrorInstance;
extern OBJECT_INSTANCE pJavaLangIllegalMonitorStateExceptionInstance;
extern OBJECT_INSTANCE pJavaLangStackOverflowErrorInstance;
extern OBJECT_INSTANCE pJavaLangErrorInstance;



#if LOCAL_FRAME_REGISTERS

/* if the frame registers are local variables, we still need to be able to access them from elsewhere.
 */

extern FRAME_REGISTERS pFrameRegisterAccessor;

#else

/* The java frame registers are either globals or actual hardware registers, not locals within the execution loop function.
 * Note: If desired, a mix of local registers and global/hardware registers could be configured. 
 */

//TODO_ATEND: if the registers are in fact actual hardware registers we cannot use a struct but for now it's handy to put everything in the struct, it will be easy to find them all later by searching on 'frameRegisters'
extern frameRegisterStruct frameRegisters;

#endif

#endif