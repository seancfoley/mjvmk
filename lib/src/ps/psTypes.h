#ifndef PSTYPES_H
#define PSTYPES_H


#include "psParameters.h"

typedef unsigned __int64 UINT64;
typedef unsigned int UINT32;
typedef int INT32;
typedef unsigned short UINT16;
typedef short INT16;
typedef unsigned char UINT8;
typedef signed char INT8;
typedef unsigned char byte;
typedef byte *PBYTE;

typedef UINT8 BOOLEAN;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif


#endif
