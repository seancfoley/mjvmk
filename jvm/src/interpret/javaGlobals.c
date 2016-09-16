#include "javaGlobals.h"


globalRegisterStruct globalRegisters;


CLASS_INSTANCE pJavaLangObject;
CLASS_INSTANCE pJavaLangClass;
CLASS_INSTANCE pJavaLangString;
CLASS_INSTANCE pJavaLangThread;
CLASS_INSTANCE pComMjvmkStackTrace;
CLASS_INSTANCE pJavaLangStringArrayClass;
CLASS_INSTANCE pCharArrayClass;
CLASS_INSTANCE pIntegerArrayClass;
CLASS_INSTANCE pShortArrayClass;

LOADED_CLASS_DEF pJavaLangObjectDef;
LOADED_CLASS_DEF pJavaLangStringDef;
ARRAY_CLASS_DEF pCharArrayClassDef;
LOADED_CLASS_DEF pJavaLangClassDef;
LOADED_CLASS_DEF pJavaLangThreadDef;

OBJECT_INSTANCE pJavaLangOutOfMemoryErrorInstance;
OBJECT_INSTANCE pJavaLangIllegalMonitorStateExceptionInstance;
OBJECT_INSTANCE pJavaLangStackOverflowErrorInstance;
OBJECT_INSTANCE pJavaLangErrorInstance;

#if LOCAL_FRAME_REGISTERS

FRAME_REGISTERS pFrameRegisterAccessor;

#else

frameRegisterStruct frameRegisters;

#endif



