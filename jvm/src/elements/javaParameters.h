
#ifndef JAVAPARAMETERS_H
#define JAVAPARAMETERS_H


#include "psParameters.h"
#include "psJavaParameters.h"



/* set to TRUE to have the registers defined as local stack variables within the main excecution loop; 
 * otherwise they will be global variables, or even hardware registers in specific architectures
 */
#define LOCAL_FRAME_REGISTERS YES

/* whether floating point numbers and operations are supported */
#define IMPLEMENTS_FLOAT YES

/* whether the platform supports 64 bit integers as defined in psTypes.h */
#define NATIVE_LONG YES

/* capacity of the class table for each application */
#define CLASS_TABLE_SIZE 256

/* if you wish the java stacks to grow in size as needed, set to TRUE */
#define GROW_STACK YES


/* the java stack will grow in increments of the following length, in java fields.
 * Only applicable if GROW_STACK is YES
 */
#define JAVA_STACK_BLOCK_SIZE 256 /* in stack fields, not bytes.  A single stack field can hold either an object or a 32-bit integer */



/* Set the following to determine if you wish to check for stack overflow with each invoke.
 * This allows you to avoid the performance hit if you are certain that the stack size is adequate in all cases.
 * If there is no stack checking and the stack size is inadequate, the system will likely crash.
 *
 * Only applicable if GROW_STACK is NO.
 */
#define CHECK_STACK YES



/* The size of the first stack segment (and the only segment, if GROW_STACK is NO) 
 * Must be at least as large as two minimal size frames (frames with no locals, no parameters, 
 * no operand stack and not synchronized).  A minimal size frame is represented by the frameStruct structure.
 */
#define INITIAL_JAVA_STACK_SIZE 256 /* in stack fields, not bytes.  A single stack field can hold either an object poointer or a 32-bit integer */

/* size of the string pools */
#define UTF8_STRING_TABLE_SIZE 256 /* this value must be less than or equal to 4096 */

#define INTERN_STRING_TABLE_SIZE 64

#define CLASS_PATH_SEPARATOR ';'



//TODO PROCESS MODEL: remember to free up any code and handlers when freeing up classes in the process model

/* if you wish the class def and constant pool to be part of the same block, then set the following to yes */
#define REALLOCATE_CONSTANT_POOL NO

#define REALLOCATE_CODE NO

/* Be careful: due to the small size of these maps (typically 1 byte), 
 * reallocating them might clog up the free list with small blocks 
 */
#define REALLOCATE_METHOD_PARAMETER_MAPS NO

#define REALLOCATE_HANDLERS NO

/* if you wish the class def and class instance to be part of the same block */
#define PREPARE_WHILE_LOADING YES

/* This makes the class loader link native methods.  Otherwise, native methods are linked when first called. */
#define EARLY_NATIVE_LINKING NO

/* Enable the creation of stored stack traces for exceptions.  
 * Recommended unless you do not like the preformance and memory hit. 
 */
#define CREATE_THROWABLE_STACK_TRACES YES

/* The stack traces are allocated in increments of frames.  If you have highly nested method 
 * calls then set this quite high, otherwise keep it low.  Onle applicable with CREATE_THROWABLE_STACK_TRACES
 * set to YES.
 */
#define STACK_TRACE_INCREMENTS 10



/* maintain registers which indicate the current class, the current method, and the current class' constant pool */
#define USE_CLASS_REGISTERS YES

/* determines whether the constant pools are altered during runtime (after loading) for better performance.  
 * This is highly recommended.
 */
#define RUN_TIME_CONSTANT_POOL_RESOLUTION YES

/* Determines whether the code stream is altered during runtime for better performance.  
 * This is very highly recommended. 
*/
#define QUICKENING_ENABLED YES




#define START_CLASS_PROPERTY_KEY "mjvmk.start.class"

#define START_ARGS_PROPERTY_KEY "mjvmk.start.args"



#define CLDC_DIR "c:\\projects\\mjvmk\\jvm\\api"

/* set this for the class path to be determined at compile time */
#define COMPILE_TIME_CLASS_PATH CLDC_DIR ";" "c:\\projects\\mjvmk\\jvm\\test"




/* whether the garbage collector computes the locations of objects in the local variable array and 
 * on the operand stack for each currently executing method.  These maps can be provided within the class
 * file if the pre-verifier is used - in which case this option should be selected.  If this option is not 
 * selected, stack frames are larger in order to separate objects from primitive types.
 */
#define USING_GC_STACK_MAPS NO


/* class monitors are created at load time for all classes and kept throughout */
#define KEEP_CLASS_MONITORS NO




#endif
