#ifndef PSCONFIG_H
#define PSCONFIG_H

#include "psTypes.h"


#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>

/* These macros and types can defer to their standard C counterparts,
 * and should defer if a standard C lib exists.
 *
 * They are the only use of the standard C functions or any other library 
 * in the non-platform-specific code, which is the vast majority of the code and
 * includes everything outside the ps directory.
 *
 */
#define memoryCompare(x, y, z) memcmp(x, y, z)
#define memoryCopy(x, y, z) memcpy(x, y, z)
#define memoryMove(x, y, z) memmove(x, y, z)
#define memorySet(x, y, z) memset(x, y, z)
#define stringLength(x) ((UINT16) strlen(x))
#define stringChar(x, y) strchr(x, y)
#define stringPrintf(x) sprintf x
#define integerToString(x, y, z) _itoa(x, y, z)
#define structOffsetOf(x, y) offsetof(x, y)
#define setJump(x) setjmp(x)
#define longJump(x, y) longjmp(x, y)
typedef jmp_buf jumpBufferStruct, *JUMP_BUFFER;


/* general purpose function return values 
 * The SC_SUCCESS code is zero, so the following axioms can be taken advantage of:
 * 1. SC_SUCCESS | SC_SUCCESS == SC_SUCCESS
 * 2. ((SC_SUCCESS | ret) != SC_SUCCESS) is equivalent to ret != SC_SUCCESS
 * 3. SUCCESS_CODE doFirstThing(); 
 *    SUCCESS_CODE doSecondThing(); 
 *    (doFirstThing() || doSecondThing()); 
 *    Above will only execute doSecondThing() if doFirstThing() returns SC_SUCCESS
 */

typedef enum successCodeEnum {
	SC_SUCCESS = 0,
	SC_FAILURE = -1
} successCodeEnum, SUCCESS_CODE;

#if TESTING

/* there are a few asserts here and there, this function is called when an assertion fails */

void assertFailure(char *str, char *file, int line, char *compilationDate, char *compilationTime);

#endif


/* The psSleep function is meant to save power if the device has such an option.
 * Whenever the system is idle this function will be called.
 */
void psSleep();

SUCCESS_CODE psInitializeHeap(void **pHeap, size_t heapSize);

SUCCESS_CODE psInitializeCPU();

void setTickISR();

#endif

