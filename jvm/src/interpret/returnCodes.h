#ifndef RETURNCODES_H
#define RETURNCODES_H

#include "types/config.h"

/*
 * The return codes indicate what type of problem occurred such that
 * the appropriate java throwable object can be thrown (if required)
 *
 */
typedef enum returnCodeEnum {
    
    SUCCESS = SC_SUCCESS,

    /* the following correspond directly to J2SE error classes */

    ERROR_CODE_ABSTRACT_METHOD,
    ERROR_CODE_CIRCULARITY,
    ERROR_CODE_EXCEPTION_IN_INITIALIZER,
    ERROR_CODE_ILLEGAL_ACCESS,              /* resolution: field or method not accessible */
    ERROR_CODE_INCOMPATIBLE_CLASS_CHANGE,   /* a class change incompatible with a previously compiled class */
    ERROR_CODE_INSTANTIATION,
    ERROR_CODE_NO_CLASS_DEF_FOUND,          /* class or interface not found */
    ERROR_CODE_NO_SUCH_FIELD,               /* field lookup failed */
    ERROR_CODE_NO_SUCH_METHOD,              /* method lookup failed */
    ERROR_CODE_OUT_OF_MEMORY,               /* not enough memory */
    ERROR_CODE_STACK_OVERFLOW,              /* ran out of stack space */
    ERROR_CODE_UNSATISFIED_LINK,
    ERROR_CODE_UNSUPPORTED_CLASS_VERSION,   /* unsupported class version */
    ERROR_CODE_VERIFY,                      /* linking: verifier found error in class */
    ERROR_CODE_VIRTUAL_MACHINE,             /* a virtual machine error see VM spec 6.3 for description */    
    ERROR_CODE_CLASS_FORMAT,
    
    /* subtypes of virtual machine error */

    ERROR_CODE_CLASS_TABLE_FULL,

    /* subtypes of class format error */

    ERROR_CODE_INVALID_FLAGS,                   /* invalid flags for class, field or method */
    ERROR_CODE_INVALID_SUPER_CLASS,
    ERROR_CODE_INVALID_SUPER_INTERFACE,
    ERROR_CODE_INVALID_FILE_SIZE,               /* reached end of file */
    ERROR_CODE_FILE_READ_ERROR,                 /* read error from file */
    ERROR_CODE_INCOMPLETE_CONSTANT_POOL_ENTRY,  /* A constant pool entry did not have its required subsequent entry */
    ERROR_CODE_INVALID_CONSTANT_POOL_ENTRY,     /* what was expected in the constant pool was not there */
    ERROR_CODE_INVALID_MEMBER_REF,              /* a bad reference to a field or method found in the constant pool of a class file */
    ERROR_CODE_DUPLICATE_METHOD,
    ERROR_CODE_DUPLICATE_FIELD,
    ERROR_CODE_OVERRIDDEN_FINAL_METHOD,
    ERROR_CODE_NO_METHOD_CODE,                  /* the code length of a method was zero, or no code attribute was found */
    ERROR_CODE_INVALID_UTF8_STRING,             /* UTF8 strings that were found in the class file were not valid */
    ERROR_CODE_INVALID_NAME,
    ERROR_CODE_INVALID_MAGIC_NUMBER,
    ERROR_CODE_INVALID_SELF_REFERENCE,          /* the class name in the constant pool does not match the file name */
    ERROR_CODE_INVALID_STATIC_CONSTANT,         /* an invalid public static final field in a class */
    ERROR_CODE_INVALID_ATTRIBUTE,               /* an attribute in the class file is not quite right */
    ERROR_CODE_INVALID_ARRAY_TYPE,
    ERROR_CODE_INVALID_FIELD_TYPE,
    ERROR_CODE_INVALID_METHOD_TYPE

} returnCodeEnum, RETURN_CODE;



typedef enum exceptionCodeEnum {

    EXCEPTION_CODE_NULL_POINTER, 
    EXCEPTION_CODE_CLASS_CAST,
    EXCEPTION_CODE_ARRAY_INDEX_OUT_OF_BOUNDS,
    EXCEPTION_CODE_ARITHMETIC,
    EXCEPTION_CODE_NEGATIVE_ARRAY_SIZE,
    EXCEPTION_CODE_INTERRUPTED,
    EXCEPTION_CODE_INSTANTIATION,
    EXCEPTION_CODE_ILLEGAL_ACCESS,
    EXCEPTION_CODE_ARRAY_STORE,
    EXCEPTION_CODE_ILLEGAL_ARGUMENT,
    EXCEPTION_CODE_ILLEGAL_MONITOR_STATE,
    EXCEPTION_CODE_ILLEGAL_THREAD_STATE,
    EXCEPTION_CODE_STRING_INDEX_OUT_OF_BOUNDS,
    EXCEPTION_CODE_IO

} exceptionCodeEnum, EXCEPTION_CODE;

#endif