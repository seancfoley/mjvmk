#ifndef BASE_H
#define BASE_H

#include "thread/thread.h"
#include "members.h"


typedef struct classInstanceStruct classInstanceStruct, *CLASS_INSTANCE;
typedef struct arrayClassDefStruct arrayClassDefStruct, *ARRAY_CLASS_DEF;
typedef struct commonClassDefStruct commonClassDefStruct, *COMMON_CLASS_DEF;
typedef struct commonLoadedDefStruct commonLoadedDefStruct, *COMMON_LOADED_DEF;
typedef union constantPoolEntryUnion constantPoolEntryUnion, *CONSTANT_POOL_ENTRY, *CONSTANT_POOL;
typedef struct classEntryStruct classEntryStruct, *CLASS_ENTRY;
typedef struct classTableStruct classTableStruct, *CLASS_TABLE;
typedef struct javaThreadStruct javaThreadStruct, *JAVA_THREAD;


typedef struct commonClassDefStruct {
    NamePackage key;
    UINT16 accessFlags;
    LOADED_CLASS_DEF pSuperClass;
    UINT16 virtualMethodCount; /* protected or public non-constructor non-static methods, including those that are final */
    METHOD_LIST pInstanceMethods;
    UINT16 tableIndex;               /* index into the class table */    
    /* number of instance object fields (as opposed to instance primitive fields), 
     * including those defined by super classes.   
     * Set to 0 for array classes and for java.lang.Class. 
     */
    UINT16 objectInstanceFieldCount; 
} commonClassDefStruct;

#define getObjectInstanceFieldCount(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->objectInstanceFieldCount)
#define getClassTableIndexFromClass(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->tableIndex)
#define getInstanceMethodList(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->pInstanceMethods)
#define getInstanceMethod(pClassDef, index) getMethod(getInstanceMethodList(pClassDef), index)
#define getParent(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->pSuperClass)
#define isArrayClass(pClassDef) (getPackageKey(pClassDef) & ARRAY_DEPTH_MASK)
#define isPrimitiveArrayClass(pClassDef) ((getPackageKey(pClassDef) & PRIMITIVE_BIT_AND_DEPTH_MASK) == PRIMITIVE_ARRAY_MASK)
#define isLoadedClass(pClassDef) (!isArrayOrInterface(pClassDef))
#define isLoadedClassOrInterface(pClassDef) (!isArrayClass(pClassDef))
#define isInterface(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->accessFlags & ACC_INTERFACE)
#define isNotInterface(pClassDef) (!isInterface(pClassDef))
#define isArrayOrInterface(pClassDef) (isArrayClass(pClassDef) || isInterface(pClassDef))
#define isPublicClass(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->accessFlags & ACC_PUBLIC)
#define isAbstractClass(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->accessFlags & ACC_ABSTRACT)
#define isSuperSpecialClass(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->accessFlags & ACC_SUPER)
#define classHasInitializer(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->accessFlags & ACC_INIT)
#define getClassKey(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->key)
#define getPackageKey(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->key.np.packageKey)
#define getNameKey(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->key.np.nameKey)
#define getNamePackageKey(pClassDef) (((COMMON_CLASS_DEF) (pClassDef))->key.namePackageKey)

#define getLoadedClassDef(pClassDef) (FIRST_ASSERT(isLoadedClassOrInterface(pClassDef)) (LOADED_CLASS_DEF) pClassDef)
#define getLoadedInterfaceDef(pClassDef) (FIRST_ASSERT(isInterface(pClassDef)) (LOADED_INTERFACE_DEF) pClassDef)
#define getArrayClassDef(pClassDef) (FIRST_ASSERT(isArrayClass(pClassDef)) (ARRAY_CLASS_DEF) pClassDef)

struct commonLoadedDefStruct {
    CONSTANT_POOL pConstantPool;

    /* includes ALL super-interfaces, not just those listed in the class file */ 
    INTERFACE_LIST pSuperInterfaceList;

    FIELD_LIST pStaticFields;
    UINT16 objectStaticFieldCount;
    
    /* the <clinit> method will always be last in the list, 
     * the only one in the list if this structure represents an interface,
     * and will not be counted as part of the list length
     */
    METHOD_LIST pStaticMethods; 
};

struct loadedClassDefStruct {
    commonClassDefStruct header;
    commonLoadedDefStruct loadedHeader;
    FIELD_LIST pInstanceFields;
    UINT16 instanceSize;
};

struct loadedInterfaceDefStruct {
    commonClassDefStruct header;
    commonLoadedDefStruct loadedHeader;
};

#define getObjectStaticFieldCount(pClassDef)  (((LOADED_CLASS_DEF) (pClassDef))->loadedHeader.objectStaticFieldCount)
#define getInstanceFieldList(pClassDef) (((LOADED_CLASS_DEF) (pClassDef))->pInstanceFields)
#define getInstanceFieldDef(pClassDef, index) getFieldDef(getInstanceFieldList(pClassDef), index)
#define getStaticFieldList(pClassDef) (((LOADED_CLASS_DEF) (pClassDef))->loadedHeader.pStaticFields)
#define getStaticFieldDef(pClassDef, index) getFieldDef(getStaticFieldList(pClassDef), index)
#define getStaticMethodList(pClassDef) (((LOADED_CLASS_DEF) (pClassDef))->loadedHeader.pStaticMethods)
#define getStaticMethod(pClassDef, index) getMethod(getStaticMethodList(pClassDef), index)
#define getLoadedClassStructSize() sizeof(loadedClassDefStruct)
#define getLoadedInterfaceStructSize() sizeof(loadedInterfaceDefStruct)
#define getLoadedClassHeader(pClassDef) (&((LOADED_CLASS_DEF) pClassDef)->header)
#define getLoadedClassParent(pClassDef) getParent(getLoadedClassHeader(pClassDef))
#define getInterfaceList(pClassDef) ((pClassDef)->loadedHeader.pSuperInterfaceList)
#define getInterfaceListLength(pClassDef) ((pClassDef)->loadedHeader.pSuperInterfaceList->length)
#define getSuperInterface(pClassDef, index) (((pClassDef)->loadedHeader.pSuperInterfaceList->pInterfaceDefs)[index])
#define getClinitMethod(pClassDef) (getStaticMethodList((LOADED_CLASS_DEF) pClassDef)->ppMethods[getStaticMethodList((LOADED_CLASS_DEF) pClassDef)->length])
#define getInstantiateSize(pClassDef) (((LOADED_CLASS_DEF) pClassDef)->instanceSize)

struct arrayClassDefStruct {
    commonClassDefStruct header;
    union {
        ARRAY_TYPE primitiveType;
        COMMON_CLASS_DEF pElementClass;            
    } elementClass;
    UINT8 elementByteSize;
};

#define getArrayClassStructSize() sizeof(arrayClassDefStruct)
#define getElementClass(pClassDef) (FIRST_ASSERT(!isPrimitiveArrayClass(&pClassDef->header)) pClassDef->elementClass.pElementClass) 
#define getPrimitiveArrayType(pClassDef) (FIRST_ASSERT(isPrimitiveArrayClass(&pClassDef->header)) pClassDef->elementClass.primitiveType) 

struct commonInstanceStruct {
    MONITOR pMonitor;
    CLASS_INSTANCE pClassInstance;
    COMMON_CLASS_DEF pClassDef;
};

typedef enum classInitializationStatusEnum {
    CLASS_INIT_STATUS_UNINITIALIZED = 0,
    CLASS_INIT_STATUS_INITIALIZED,
    CLASS_INIT_STATUS_ERRONEOUS
} classInitializationStatusEnum, CLASS_INIT_STATUS;

/* Note: When putting objects of type class on the stack, 
 * the stack will point to the instance member of this structure.
 */
struct classInstanceStruct {
    commonInstanceStruct instance;
    UINT8 initializationStatus;
    JAVA_THREAD pInitializingThread; /* thread which runs the <clinit> */
    COMMON_CLASS_DEF pRepresentativeClassDef; /* the class that this instance of java.lang.Class represents */
    
    /* the object fields appear first and the primitives follow, 
     * unlike objects where the objects lie above and the primitives below 
     */
    jobjectField objectStaticFields[1];
};

#define getStaticPrimitiveIndexOffset(numObjectFields) sizeToPrimitiveFieldSize((numObjectFields) * getObjectFieldSize())
#define getClassInstanceSize(numPrimitiveFields, numObjectFields) (structOffsetOf(classInstanceStruct, objectStaticFields) + (getStaticPrimitiveIndexOffset(numObjectFields) + (numPrimitiveFields)) * getPrimitiveFieldSize())
#define getObjectStaticFields(pClassInstance) ((pClassInstance)->objectStaticFields)
#define getObjectStaticField(pClassInstance, index) (getObjectStaticFields(pClassInstance) + index)
#define getPrimitiveStaticField(pClassInstance, index) (((JPRIMITIVE_FIELD) getObjectStaticFields(pClassInstance)) + index)
#define getClassMonitor(pClassInstance) ((pClassInstance)->instance.pMonitor)
#define getClassMonitorPtr(pClassInstance) (&(pClassInstance)->instance.pMonitor)
#define classIsInitialized(pClassInstance, pCurrentThread) ((pClassInstance)->initializationStatus == CLASS_INIT_STATUS_INITIALIZED || (pClassInstance)->pInitializingThread == (pCurrentThread))
#define classInitializationHasStarted(pClassInstance) ((pClassInstance)->pInitializingThread != NULL)
#define classInitializationFailed(pClassInstance) ((pClassInstance)->initializationStatus == CLASS_INIT_STATUS_ERRONEOUS)
#define extractClassObjectFromClassInstance(pClassInstance) ((pClassInstance)->instance)
#define getClassObjectFromClassInstance(pClassInstance) ((OBJECT_INSTANCE) (&(pClassInstance)->instance))
#define getClassInstanceFromClassObject(pObject)    ((CLASS_INSTANCE) (((PBYTE) pObject) - structOffsetOf(classInstanceStruct, instance)))
#define setClassInitializingThread(pClassInstance, pThread) ((pClassInstance)->pInitializingThread = (pThread))
#define setClassInitStatus(pClassInstance, value) ((pClassInstance)->initializationStatus = (value))


/* object fields lie above the header */
struct objectInstanceStruct {
    commonInstanceStruct header;
    jprimitiveField primitiveFields[1];
};

#define getObjectFieldCount(pObject) getObjectInstanceFieldCount((pObject)->header.pClassDef)
#define getFieldDataSize(totalObjectSize) ((totalObjectSize) - structOffsetOf(objectInstanceStruct, primitiveFields))
#define getObjectInstanceSize(numPrimitiveFields, numObjectFields) (structOffsetOf(objectInstanceStruct, primitiveFields) + (numPrimitiveFields) * getPrimitiveFieldSize() + (numObjectFields) * getObjectFieldSize()) 
#define getPrimitiveFields(pObject) ((pObject)->primitiveFields)
#define getPrimitiveField(pObject, index) (getPrimitiveFields(pObject) + (index))
#define getObjectFields(pObject) (((JOBJECT_FIELD) (pObject)) - 1)
#define getObjectField(pObject, index) (getObjectFields(pObject) - (index))
#define getObjectMonitor(pObject) ((pObject)->header.pMonitor)
#define getObjectMonitorPtr(pObject) (&(pObject)->header.pMonitor)
#define isArrayObject(pObject) isArrayClass((pObject)->header.pClassDef)
#define isPrimitiveArrayObject(pObject) isPrimitiveArrayClass((pObject)->header.pClassDef)


typedef struct arrayInstanceHeaderStruct {
    commonInstanceStruct header;
    UINT16 arrayLength;
} arrayInstanceHeaderStruct;

typedef struct objectArrayInstanceStruct {
    arrayInstanceHeaderStruct header;
    jobjectField values[];
} objectArrayInstanceStruct, *OBJECT_ARRAY_INSTANCE;

typedef struct booleanArrayInstanceStruct {
    arrayInstanceHeaderStruct header;
    jboolean values[];
} booleanArrayInstanceStruct, *BOOLEAN_ARRAY_INSTANCE;

typedef struct byteArrayInstanceStruct {
    arrayInstanceHeaderStruct header;
    jbyte values[];
} byteArrayInstanceStruct, *BYTE_ARRAY_INSTANCE;

typedef struct charArrayInstanceStruct {
    arrayInstanceHeaderStruct header;
    jchar values[];
} charArrayInstanceStruct, *CHAR_ARRAY_INSTANCE;

typedef struct shortArrayInstanceStruct {
    arrayInstanceHeaderStruct header;
    jshort values[];
} shortArrayInstanceStruct, *SHORT_ARRAY_INSTANCE;

typedef struct intArrayInstanceStruct {
    arrayInstanceHeaderStruct header;
    jint values[];
} intArrayInstanceStruct, *INT_ARRAY_INSTANCE;

typedef struct longArrayInstanceStruct {
    arrayInstanceHeaderStruct header;
    jlong values[];
} longArrayInstanceStruct, *LONG_ARRAY_INSTANCE;

typedef INT_ARRAY_INSTANCE STACK_TRACE_METHOD_ARRAY;
typedef SHORT_ARRAY_INSTANCE STACK_TRACE_CLASS_ARRAY;

#define getObjectArrayElement(pObject, index) getObjectFieldObject(((OBJECT_ARRAY_INSTANCE) (pObject))->values + index)
#define getObjectArrayField(pObject, index) (((OBJECT_ARRAY_INSTANCE) (pObject))->values + index)
#define getBooleanArrayElement(pObject, index) (((BOOLEAN_ARRAY_INSTANCE) (pObject))->values[index])
#define getByteArrayElement(pObject, index) (((BYTE_ARRAY_INSTANCE) (pObject))->values[index])
#define getCharArrayElement(pObject, index) (((CHAR_ARRAY_INSTANCE) (pObject))->values[index])
#define getShortArrayElement(pObject, index) (((SHORT_ARRAY_INSTANCE) (pObject))->values[index])
#define getIntArrayElement(pObject, index) (((INT_ARRAY_INSTANCE) (pObject))->values[index])
#define getLongArrayElement(pObject, index) (((LONG_ARRAY_INSTANCE) (pObject))->values[index])

#define setObjectArrayElement(pObject, index, x) setObjectFieldObject(((OBJECT_ARRAY_INSTANCE) (pObject))->values + index, (x))
#define setBooleanArrayElement(pObject, index, x) (((BOOLEAN_ARRAY_INSTANCE) (pObject))->values[index] = (x))
#define setByteArrayElement(pObject, index, x) (((BYTE_ARRAY_INSTANCE) (pObject))->values[index] = (x))
#define setCharArrayElement(pObject, index, x) (((CHAR_ARRAY_INSTANCE) (pObject))->values[index] = (x))
#define setShortArrayElement(pObject, index, x) (((SHORT_ARRAY_INSTANCE) (pObject))->values[index] = (x))
#define setIntArrayElement(pObject, index, x) (((INT_ARRAY_INSTANCE) (pObject))->values[index] = (x))
#define setLongArrayElement(pObject, index, x) (((LONG_ARRAY_INSTANCE) (pObject))->values[index] = (x))



#if IMPLEMENTS_FLOAT

typedef struct floatArrayInstanceStruct {
    arrayInstanceHeaderStruct header;
    jfloat values[];
} floatArrayInstanceStruct, *FLOAT_ARRAY_INSTANCE;

typedef struct doubleArrayInstanceStruct {
    arrayInstanceHeaderStruct header;
    jdouble values[];
} doubleArrayInstanceStruct, *DOUBLE_ARRAY_INSTANCE;

#define getFloatArrayElement(pObject, index) (((FLOAT_ARRAY_INSTANCE) (pObject))->values[index])
#define getDoubleArrayElement(pObject, index) (((DOUBLE_ARRAY_INSTANCE) (pObject))->values[index])
#define setFloatArrayElement(pObject, index, x) (((FLOAT_ARRAY_INSTANCE) (pObject))->values[index] = (x))
#define setDoubleArrayElement(pObject, index, x) (((DOUBLE_ARRAY_INSTANCE) (pObject))->values[index] = (x))

#endif

/* generic array object struct */
struct arrayInstanceStruct {
    arrayInstanceHeaderStruct header;
    jobjectField firstValue;
};

#define getByteSizeOfArrayInstance(length, elementByteSize) (length * elementByteSize + structOffsetOf(arrayInstanceStruct, firstValue))  
#define getArrayLength(pArrayInstance) (((ARRAY_INSTANCE) pArrayInstance)->header.arrayLength)

 
/* Representations of internally accessed objects */


/* java.lang.String */
typedef struct stringInstanceStruct { /* matches the fields in the java.lang.String definition */
    jobjectField pCharArray;
    commonInstanceStruct header;
    jprimitiveField length;
    jprimitiveField offset;
} stringInstanceStruct, *STRING_INSTANCE;

typedef struct internedStringInstanceStruct internedStringInstanceStruct, *INTERNED_STRING_INSTANCE;

struct internedStringInstanceStruct {
    jobjectField pCharArray;
    commonInstanceStruct header;
    jprimitiveField length;
    jprimitiveField offset;
    INTERNED_STRING_INSTANCE next;
};

#define getStringObjectFieldCount() 1
#define getInternedStringInstanceSize() sizeof(internedStringInstanceStruct)
#define getStringInstanceFromObjectInstance(pObject) ((STRING_INSTANCE) (((JOBJECT_FIELD) (pObject)) - getStringObjectFieldCount()))
#define getInternedStringInstanceFromObjectInstance(pObject) ((INTERNED_STRING_INSTANCE) (((JOBJECT_FIELD) (pObject)) - getStringObjectFieldCount()))

/* java.lang.Throwable */
typedef struct throwableInstanceStruct {
    jobjectField stackTrace;
    jobjectField messageString;
    commonInstanceStruct header;
} throwableInstanceStruct, *THROWABLE_INSTANCE;

#define getThrowableInstanceFromObjectInstance(pObject) ((THROWABLE_INSTANCE) (((JOBJECT_FIELD) (pObject)) - 2))

/* java.lang.Thread */
typedef struct threadInstanceStruct {
    jobjectField javaThreadStructObject;
    jobjectField runnable;
    commonInstanceStruct header;
} threadInstanceStruct, *THREAD_INSTANCE;

#define getThreadInstanceFromObjectInstance(pObject) ((THREAD_INSTANCE) (((JOBJECT_FIELD) (pObject)) - 2))

/* com.mjvmk.StackTrace */
typedef struct stackTraceInstanceStruct {
    jobjectField nextStackTraceInstance;
    jobjectField stackTraceMethodArray;
    jobjectField stackTraceClassArray;
    commonInstanceStruct header;
} stackTraceInstanceStruct, *STACK_TRACE_INSTANCE;

#define getStackTraceInstanceFromObjectInstance(pObject) ((STACK_TRACE_INSTANCE) (((JOBJECT_FIELD) (pObject)) - 3))

/* java.lang.ExceptionInInitializerError */
typedef struct exceptionInInitializerErrorInstanceStruct {
    jobjectField throwable;
    throwableInstanceStruct baseMembers;
} exceptionInInitializerErrorInstanceStruct, *EXCEPTION_IN_INITIALIZER_INSTANCE;

#define getExceptionInInitializerErrorInstanceFromObjectInstance(pObject) ((EXCEPTION_IN_INITIALIZER_INSTANCE) (((JOBJECT_FIELD) getThrowableInstanceFromObjectInstance(pObject)) - 1))

/* java.util.Vector */
typedef struct vectorInstanceStruct {
    jobjectField objectArray;
    commonInstanceStruct header;
    jprimitiveField size;
} vectorInstanceStruct, *VECTOR_INSTANCE;

#define getVectorInstanceFromObjectInstance(pObject) ((VECTOR_INSTANCE) (((JOBJECT_FIELD) (pObject)) - 1))

/* com.mjvmk.ResourceInputStream */
typedef struct resourceInputStreamStruct {
    jobjectField fileAccess;
    commonInstanceStruct header;
    jprimitiveField isClosed;
} resourceInputStreamStruct, *RESOURCE_INPUT_STREAM_INSTANCE;

#define getResourceInputStreamInstanceFromObjectInstance(pObject) ((RESOURCE_INPUT_STREAM_INSTANCE) (((JOBJECT_FIELD) (pObject)) - 1))


/* class table structures and macros */


typedef enum returnCodeEnum ERROR_CODE;

typedef enum classStatusEnum {
    CLASS_STATUS_ERROR = -1,
    CLASS_STATUS_INEXISTENT = 0, /* this one must be zero since all class table entries start out that way */
    CLASS_STATUS_REFERENCED,
    CLASS_STATUS_LOADED,
    CLASS_STATUS_PREPARED
} classStatusEnum, CLASS_STATUS;

/* in order to prepare the class, we need access to the monitor, which is why this structure is required in the class table */
typedef struct classLoadEntryStruct {
    COMMON_CLASS_DEF pClassDef;
    MONITOR pMonitor;
} classLoadEntryStruct, *CLASS_LOAD_ENTRY;

/* a class table entry */
struct classEntryStruct {
    NamePackageKey key;
    CLASS_STATUS status;
    /* The status is an indicator of what is found below in the union.
     * The error status indicates a failed load attempt - and hence any further attempts to load should be abandoned. 
     */
    union entryUnion {
        ERROR_CODE error;               /* status == CLASS_STATUS_ERROR */
        MONITOR pMonitor;               /* status == CLASS_STATUS_REFERENCED */
        CLASS_LOAD_ENTRY pLoad;         /* status == CLASS_STATUS_LOADED */
        CLASS_INSTANCE pClassInstance;  /* status == CLASS_STATUS_PREPARED */
    } u;
};

#define getClassDefFromClassEntry(pClassEntry) (FIRST_ASSERT((pClassEntry)->status >= CLASS_STATUS_LOADED)          \
                                                ((pClassEntry)->status == CLASS_STATUS_LOADED) ?                    \
                                                    (pClassEntry)->u.pLoad->pClassDef :                             \
                                                    (pClassEntry)->u.pClassInstance->pRepresentativeClassDef)
#define getClassMonitorPtrFromEntry(pClassEntry) (FIRST_ASSERT((pClassEntry)->status == CLASS_STATUS_REFERENCED)    \
                                                    &(pClassEntry)->u.pMonitor)
#define classInTableIsLoaded(pClassEntry) ((pClassEntry)->status >= CLASS_STATUS_LOADED)
#define classInTableIsPrepared(pClassEntry) ((pClassEntry)->status >= CLASS_STATUS_PREPARED)
#define classInTableIsInError(pClassEntry) ((pClassEntry)->status <= CLASS_STATUS_ERROR)

struct classTableStruct {
    UINT16 size;
    UINT16 count;
    classEntryStruct entries[];
};

#define calculateClassTableSize() (CLASS_TABLE_SIZE * sizeof(classEntryStruct) + structOffsetOf(classTableStruct, entries))
#define getClassTableSize(pClassTable)                  ((pClassTable)->size)
#define getClassTableBase(pClassTable)                  ((pClassTable)->entries)
#define getClassTableCount(pClassTable)                 ((pClassTable)->count)
#define incrementClassTableCount(pClassTable)           ((pClassTable)->count++)
#define setClassTableSize(pClassTable, x)               ((pClassTable)->size = (UINT16) (x))
#define getClassTableIndex(pClassTable, pClassEntry)    (pClassEntry - getClassTableBase(pClassTable))
#define getEntryInTableByIndex(pClassTable, index)      (getClassTableBase(pClassTable)[index])
#define getEntryPtrInTableByIndex(pClassTable, index)   (getClassTableBase(pClassTable) + index)




#endif
