#ifndef XTYPES_H
#define XTYPES_H

#include "types/config.h"
#include "psTypes.h"
#include "javaParameters.h"

/* data types that will store their java counterparts 
 * Integers in java are two's complement so the following types are all unsigned
 */
typedef UINT8 jbyte;
typedef UINT16 jshort;
typedef UINT16 jchar;
typedef UINT32 jint;

/* In the JVM, booleans are stored as bytes */
typedef jbyte jboolean;

#if IMPLEMENTS_FLOAT
typedef double *JDOUBLE;
typedef double jdouble;
typedef float *JFLOAT;
typedef float jfloat;
#endif

#if NATIVE_LONGS

typedef UINT64 jlong;

#define setJlong(longVar, highInt, lowInt)          (longVar = (((UINT64) highInt) << 32) + (lowInt))

/* long comparisons */
#define jlongLessThan(longVar1, longVar2)           ((longVar1) < (longVar2))
#define jlongGreaterThan(longVar1, longVar2)        ((longVar1) > (longVar2))

/* long conversions */
#define jlong2Int(longValue)                        ((UINT32) ((INT32) longValue))
#define jint2Long(longVar, intValue)                ((UINT64) ((INT32) intValue) >> 31 + (INT32) intValue))


/* long operations */
#define jlongIncrement(longVar1, longVar2)          ((longVar1) += (longVar2))
#define jlongDecrement(longVar1, longVar2)          ((longVar1) -= ((longVar2))
#define jlongNegate(longVar)                        ((longVar) = -(longVar))
#define jlongBitwiseEitherOr(longVar1, longVar2)    ((longVar1) ^= (longVar2))
#define jlongBitwiseOr(longVar1, longVar2)          ((longVar1) |= (longVar2))
#define jlongBitwiseAnd(longVar1, longVar2)         ((longVar1) &= (longVar2))
#define jlongMultiply(longVar1, longVar2)           ((longVar1) * (longVar2))
#define jlongDivide(longVar1, longVar2)             ((longVar1) / (longVar2))
#define jlongShiftLeft(longVar1, uintVar2)          ((longVar1) << (uintVar2))
#define jlongShiftRight(longVar1, uintVar2)         ((longVar1) >> (uintVar2))
#define jlongUnsignedShiftRight(longVar1, uintVar2) ((longVar1) >>> (uintVar2))
#define jlongRemainder(longVar1, longVar2)          ((longVar1) % (longVar2))


/* floating point operations */
#if IMPLEMENTS_FLOAT

#define jlong2Float(longVar)                        ((jfloat) longVar)
#define jlong2Double(longVar)                       ((jdouble) longVar)
#define jdouble2Long(doubleVar)                     ((jlong) doubleVar)
#define jfloat2Long(floatVar)                       ((jlong) floatVar)

#endif



#else

/* Note that we are big-endian here */
typedef struct jlong { 
    UINT32 high; 
    UINT32 low;
} jlong, *JLONG;


extern jlong jlongZero;
extern jlong jlongOne;

#define setJlong(longVar, highInt, lowInt)          (longVar.high = (highInt), longVar.low = (lowInt), longVar)

/* long comparisons */
#define jlongLessThan(longVar1, longVar2)           (((longVar1).high < (longVar2).high) || (((longVar1).high == (longVar2).high) && ((longVar1).low < (longVar2).low)))
#define jlongGreaterThan(longVar1, longVar2)        jlongLessThan(longVar2, longVar1)

/* long conversions */
#define jlong2Int(longValue)                        (longValue.low)
#define jint2Long(longVar, intValue)                (longVar.high = ((INT32) intValue) >> 31, longVar.low = (INT32) intValue, longVar)


/* long operations */
#define jlongIncrement(longVar1, longVar2)          {(longVar1).low += (longVar2).low; (longVar1).high += (longVar2).high + ((longVar1).low < (longVar2).low);}
#define jlongDecrement(longVar1, longVar2)          {(longVar1).high -= ((longVar2).high + ((longVar1).low < (longVar2).low)); (longVar1).low  -= (longVar2).low;}
#define jlongNegate(longVar)                        {(longVar).low = -((INT32) (longVar).low); (longVar).high = -((INT32) ((longVar).high + ((longVar).low != 0)));}
#define jlongBitwiseEitherOr(longVar1, longVar2)    {(longVar1).low ^= (longVar2).low; (longVar1).high ^= (longVar2).high;}
#define jlongBitwiseOr(longVar1, longVar2)          {(longVar1).low |= (longVar2).low; (longVar1).high |= (longVar2).high;}
#define jlongBitwiseAnd(longVar1, longVar2)         {(longVar1).low &= (longVar2).low; (longVar1).high &= (longVar2).high;}
jlong jlongMultiply(jlong, jlong);
jlong jlongDivide(jlong, jlong);
jlong jlongShiftLeft(jlong, UINT32);
jlong jlongShiftRight(jlong, UINT32);
jlong jlongUnsignedShiftRight(jlong, UINT32);
jlong jlongRemainder(jlong, jlong);


/* long/time conversions */
#define setTimeMillis(pTime, longMillis)                (((TIME_VALUE) (pTime))->milliseconds = (longMillis).low + ((longMillis).high << 32))
#define setTimeMillisNanos(pTime, longMillis, intNanos) (((TIME_VALUE) (pTime))->milliseconds = (longMillis).low + (((UINT64) (longMillis).high) << 32) + (((intNanos) + 999) / 1000))
#define getTime(longMillis, pTime)                      {(longMillis).high = (UINT32) ((pTime)->milliseconds >> 32); (longMillis).low = (UINT32) ((TIME_VALUE) (pTime))->milliseconds;}

/* floating point operations */
#if IMPLEMENTS_FLOAT

jfloat jlong2Float(jlong);
jdouble jlong2Double(jlong);
jlong jdouble2Long(jdouble);
jlong jfloat2Long(jfloat);

#endif

#endif

#if IMPLEMENTS_FLOAT

jfloat jfloatMod(jfloat dividend, jfloat divisor);
jdouble jdoubleMod(jdouble dividend, jdouble divisor);

#endif


typedef char ARRAY_TYPE;
typedef char FIELD_TYPE;

#define BOOLEAN_ARRAY_TYPE 'Z'
#define CHAR_ARRAY_TYPE 'C'
#define INT_ARRAY_TYPE 'I'
#define LONG_ARRAY_TYPE 'J'
#define SHORT_ARRAY_TYPE 'S'
#define BYTE_ARRAY_TYPE 'B'
#define OBJECT_ARRAY_TYPE 'L'
#define ARRAY_ARRAY_TYPE '['

#define BOOLEAN_FIELD_TYPE BOOLEAN_ARRAY_TYPE
#define CHAR_FIELD_TYPE CHAR_ARRAY_TYPE
#define INT_FIELD_TYPE INT_ARRAY_TYPE
#define LONG_FIELD_TYPE LONG_ARRAY_TYPE
#define SHORT_FIELD_TYPE SHORT_ARRAY_TYPE
#define BYTE_FIELD_TYPE BYTE_ARRAY_TYPE
#define OBJECT_FIELD_TYPE OBJECT_ARRAY_TYPE
#define ARRAY_FIELD_TYPE ARRAY_ARRAY_TYPE

#if IMPLEMENTS_FLOAT
#define FLOAT_ARRAY_TYPE 'F'
#define DOUBLE_ARRAY_TYPE 'D'
#define FLOAT_FIELD_TYPE FLOAT_ARRAY_TYPE
#define DOUBLE_FIELD_TYPE DOUBLE_ARRAY_TYPE
#endif

#define VOID_RETURN_TYPE 'V'

/* An instruction is a pointer to a sequence of bytes.
 * In the java programming language, the first byte indicates the type of the intruction,
 * unless it is the wide instruction, in which case both the first and second bytes
 * are the indicators.
 *
 * Subsequent bytes are the instruction arguments.  The number of arguments is
 * variable.
 *
 */
typedef PBYTE INSTRUCTION;


/* the types of object references, all of which may be cast into a COMMON_INSTANCE */
typedef struct commonInstanceStruct commonInstanceStruct, *COMMON_INSTANCE;
typedef struct objectInstanceStruct objectInstanceStruct, *OBJECT_INSTANCE;
typedef struct arrayInstanceStruct arrayInstanceStruct, *ARRAY_INSTANCE;


/* A field is a location for data storage of java types.
 * We have three different types of fields - primitive, object and stack.
 * Primitive holds primitive types, object holds object types, and stack holds either one.
 * An object array has object fields to store its elements.  An object has both primitive and object fields.
 * Methods have stack fields located on the stack, both in the local variable array and on the
 * operand stack of each method frame.  These stack fields may hold both primitives and objects.
 */

typedef union jprimitiveField {
        jint intValue;
#if IMPLEMENTS_FLOAT
        jfloat floatValue;
#endif
} jprimitiveField, *JPRIMITIVE_FIELD;


#define getPrimitiveFieldData(pField)               (*(pField))
#define getPrimitiveFieldInt(pField)                ((pField)->intValue)
#define getPrimitiveFieldLong(pField)               (*(JLONG) (pField))
#define setPrimitiveFieldData(pField, value)        (*(pField) = (value))
#define setPrimitiveFieldInt(pField, value)         ((pField)->intValue = (value))
#define setPrimitiveFieldLong(pField, value)        (*((JLONG) (pField)) = (value))
#define extractPrimitiveFieldInt(field)             ((field).intValue)
#define extractPrimitiveFieldLong(field)            (*(JLONG) &(field))
#define insertPrimitiveFieldInt(field, value)       ((field).intValue = (value))
#define insertPrimitiveFieldLong(field, value)      ((*(JLONG) &(field)) = (value))

#if IMPLEMENTS_FLOAT

#define getPrimitiveFieldFloat(pField)              ((pField)->floatValue)
#define getPrimitiveFieldDouble(pField)             (*(JDOUBLE) (pField))
#define setPrimitiveFieldFloat(pField, value)       ((pField)->floatValue = (value))
#define setPrimitiveFieldDouble(pField, value)      (*((JDOUBLE) (pField)) = (value))
#define extractPrimitiveFieldFloat(field)           ((field).floatValue)
#define extractPrimitiveFieldDouble(field)          (*(JDOUBLE) &(field))
#define insertPrimitiveFieldFloat(field, value)     ((field).floatValue = (value))
#define insertPrimitiveFieldDouble(field, value)    ((*(JDOUBLE) &(field)) = (value))

#endif



typedef union jobjectField {
        OBJECT_INSTANCE pObject;
        ARRAY_INSTANCE pArray;
} jobjectField, *JOBJECT_FIELD;


#define getObjectFieldObject(pField)                ((pField)->pObject)
#define getObjectFieldObjectPtr(pField)             (&(pField)->pObject)
#define setObjectFieldObject(pField, value)         ((pField)->pObject = (value))
#define extractObjectFieldObject(field)             ((field).pObject)
#define extractObjectFieldObjectPtr(field)          (&(field).pObject)
#define insertObjectFieldObject(field, value)       ((field).pObject = (value))


#if USING_GC_STACK_MAPS



typedef union jstackField {
        jint intValue;
        OBJECT_INSTANCE pObject;
        ARRAY_INSTANCE pArray;
        INSTRUCTION pReturnAddress; 
#if IMPLEMENTS_FLOAT
        jfloat floatValue;
#endif
} jstackField, *JSTACK_FIELD;


#define getStackFieldData(pField)                   (*(pField))
#define getStackFieldInt(pField)                    ((pField)->intValue)
#define getStackFieldObject(pField)                 ((pField)->pObject)
#define getStackFieldObjectPtr(pField)              (&(pField)->pObject)
#define getStackFieldRetAddress(pField)             ((pField)->pReturnAddress)
#define getStackFieldLong(pField)                   (*(JLONG) (pField))

#define setStackFieldData(pField, value)            (*(pField) = (value))
#define setStackFieldInt(pField, value)             ((pField)->intValue = (value))
#define setStackFieldObject(pField, value)          ((pField)->pObject = (value))
#define setStackFieldRetAddress(pField, value)      ((pField)->pReturnAddress = (value))
#define setStackFieldLong(pField, value)            (*((JLONG) (pField)) = (value))


#define extractStackFieldInt(field)                 ((field).intValue)
#define extractStackFieldObject(field)              ((field).pObject)
#define extractStackFieldObjectPtr(field)           (&(field).pObject)
#define extractStackFieldLong(field)                (*(JLONG) &(field))

#define insertStackFieldInt(field, value)           ((field).intValue = (value))
#define insertStackFieldLong(field, value)          ((*(JLONG) &(field)) = (value))
#define insertStackFieldObject(field, value)        ((field).pObject = (value))


#if IMPLEMENTS_FLOAT

#define getStackFieldFloat(pField)                  ((pField)->floatValue)
#define getStackFieldDouble(pField)                 (*(JDOUBLE) (pField))
#define setStackFieldFloat(pField, value)           ((pField)->floatValue = (value))
#define setStackFieldDouble(pField, value)          (*((JDOUBLE) (pField)) = (value))
#define extractStackFieldFloat(field)               ((field).floatValue)
#define extractStackFieldDouble(field)              (*(JDOUBLE) &(field))
#define insertStackFieldFloat(field, value)         ((field).floatValue = (value))
#define insertStackFieldDouble(field, value)        ((*(JDOUBLE) &(field)) = (value))

#endif


#else /* USING_GC_STACK_MAPS */

/* on the stack we must separate the objects from the primitive types, so that the garbage collector
 * knows which entries are objects and which entries are not objects.
 */

/* Note: the order is important below!  The object location must come first.  This is because when
 * methods that return objects complete, the return object is inserted into its new location on top of the
 * previous method's operand stack.  This coincides with the beginning of the current frame structure.
 * It is OK to overwrite the first element of the frame structure since it is no longer of use, but the
 * second element is used when popping the frame.  So by having the object location first below, we ensure
 * that it is the first element of the frame structure that is overwritten.
 */
typedef struct jstackField {
    jobjectField objectField;
    union {
        jprimitiveField primitiveField;
        INSTRUCTION pReturnAddress;
    } primitives;
} jstackField, *JSTACK_FIELD;


#define DOUBLE_PRIMITIVE_STACK_INDICATOR ((OBJECT_INSTANCE) 0x01)

/* note that longs and doubles always appear in the second of their two deignated stack entries.  This is so that the first entry
 * is an indicator that the second entry contains a long, and not an object.  The object ptr is set to 1 in the first entry as an indicator.
 */
#define getStackFieldData(pField)                   (*(pField))
#define getStackFieldInt(pField)                    extractPrimitiveFieldInt((pField)->primitives.primitiveField)
#define getStackFieldObject(pField)                 extractObjectFieldObject((pField)->objectField)
#define getStackFieldObjectPtr(pField)              extractObjectFieldObjectPtr((pField)->objectField)
#define getStackFieldRetAddress(pField)             ((pField)->primitives.pReturnAddress)
#define getStackFieldLong(pField)                   (*(JLONG) ((pField) + 1))

#define setStackFieldData(pField, value)            (*(pField) = (value))
#define setStackFieldInt(pField, value)             insertPrimitiveFieldInt((pField)->primitives.primitiveField, value)
#define setStackFieldObject(pField, value)          insertObjectFieldObject((pField)->objectField, value)
#define setStackFieldRetAddress(pField, value)      ((pField)->primitives.pReturnAddress = (value))
#define setStackFieldLong(pField, value)            {insertObjectFieldObject((pField)->objectField, DOUBLE_PRIMITIVE_STACK_INDICATOR); *(JLONG) ((pField) + 1) = (value);} 


#define extractStackFieldInt(field)                 extractPrimitiveFieldInt((field).primitives.primitiveField)
#define extractStackFieldObject(field)              extractObjectFieldObject((field).objectField)
#define extractStackFieldObjectPtr(field)           extractObjectFieldObjectPtr((field).objectField)
#define extractStackFieldLong(field)                (*(JLONG) (&(field) + 1))

#define insertStackFieldInt(field, value)           insertPrimitiveFieldInt((field).primitives.primitiveField, value)
#define insertStackFieldObject(field, value)        insertObjectFieldObject((field).objectField, value)
#define insertStackFieldLong(field, value)          {insertObjectFieldObject((field).objectField, DOUBLE_PRIMITIVE_STACK_INDICATOR); *(JLONG) (&(field) + 1) = (value);}


#if IMPLEMENTS_FLOAT

#define getStackFieldFloat(pField)                  extractPrimitiveFieldFloat((pField)->primitives.primitiveField)
#define getStackFieldDouble(pField)                 (*(JDOUBLE) ((pField) + 1))
#define setStackFieldFloat(pField, value)           insertPrimitiveFieldFloat((pField)->primitives.primitiveField, value)
#define setStackFieldDouble(pField, value)          {insertObjectFieldObject((pField)->objectField, DOUBLE_PRIMITIVE_STACK_INDICATOR); *(JDOUBLE) ((pField) + 1) = (value);} 
#define extractStackFieldFloat(field)               extractPrimitiveFieldFloat((field).primitives.primitiveField)
#define extractStackFieldDouble(field)              (*(JDOUBLE) (&(field) + 1))
#define insertStackFieldFloat(field, value)         insertPrimitiveFieldFloat((field).primitives.primitiveField, value)
#define insertStackFieldDouble(field, value)        {insertObjectFieldObject((field).objectField, DOUBLE_PRIMITIVE_STACK_INDICATOR); *(JDOUBLE) (&(field) + 1) = (value);}

#endif

#endif



#define SHIFT_DIVIDE_PRIMITIVE 2 /* log base 2 of sizeof(jprimitiveField) - this is 2 since ints and floats are exactly 4 bytes */
#define getPrimitiveFieldSize() (sizeof(jprimitiveField))
#define sizeToPrimitiveFieldSize(size) (((size) + getPrimitiveFieldSize() - 1) >> SHIFT_DIVIDE_PRIMITIVE)

#define getObjectFieldSize() (sizeof(jobjectField))
#define sizeToObjectFieldSize(size) (((size) + getObjectFieldSize() - 1) >> SHIFT_DIVIDE_POINTER)

#define getStackFieldSize() (sizeof(jstackField))
#define sizeToStackFieldSize(size) (((size) + getStackFieldSize() - 1) / getStackFieldSize())
#define jstackFieldSizeOf(x) sizeToStackFieldSize(sizeof(x))




#endif