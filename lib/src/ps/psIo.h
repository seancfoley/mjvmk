#ifndef PSIO_H
#define PSIO_H

#include "psConfig.h"
#include "psTypes.h"

/* the standard output and error streams */

void psPrintOut(const char *str);
void psPutChar(int c);
void psFlushOut();
void psPrintErr(const char *str);
void psPrintNErr(const char *str, UINT16 strLength);
void psPutCharErr(int c);
void psPrintBanner();


/* logging */

#if LOGGING

void logNewLine();
void logPrintf(const char *format, ...);
void logFlush();
void logChar(int c);
SUCCESS_CODE initializeLogging();

#endif


/* access to generic files, whether in a file system, romized, or loaded over a network */

/* if longjmp is not used when a file read error occurs, then the following value is what
 * the function will return, it is analogous to EOF in the standard C IO library.
 */
#define EOF_VALUE 0


typedef struct fileAccessStruct fileAccessStruct, *FILE_ACCESS;

typedef struct fileAccessMethodsStruct fileAccessMethodsStruct, *FILE_ACCESS_METHODS;

struct fileAccessStruct {
    FILE_ACCESS_METHODS pFileAccessMethods; /* functions for acessing bytes within a file */
    JUMP_BUFFER pErrorJump; /* an optional place to jump when an I/O error occurs */
    INT16 errorFlag; /* indicates an error when non-zero */
    UINT16 classPathIndex; /* indicates where on the classpath this file was obtained from */
};

/* file system access */

#if USE_FILE_SYSTEM

#include <stdio.h>

#define FILE_PATH_SEPARATOR "\\" /* if a file system is in use */

typedef struct fileSystemAccessStruct fileSystemAccessStruct, *FILE_SYSTEM_ACCESS;

/* mirrors the structure of a fileAccessStruct, represents a file in the file system */
struct fileSystemAccessStruct {
    fileAccessStruct genericFileAccess;
    FILE *fp;
    UINT8 cachedByte;
    BOOLEAN hasCachedByte;
    void (*handleReadError) (FILE_SYSTEM_ACCESS pFileAccess);
};

#define getFileSystemAccessStructSize() sizeof(fileSystemAccessStruct)

BOOLEAN psFileSystemFileOpen(const char *path, void (*handleReadError) (FILE_SYSTEM_ACCESS pFileAccess), FILE_SYSTEM_ACCESS pFileAccess);
BOOLEAN fileSystemEof(FILE_SYSTEM_ACCESS pFileAccess);
UINT8 fileSystemLoadByte(FILE_SYSTEM_ACCESS pFileAccess);
UINT16 fileSystemLoadU2(FILE_SYSTEM_ACCESS pFileAccess);
UINT32 fileSystemLoadU4(FILE_SYSTEM_ACCESS pFileAccess);
UINT32 fileSystemLoadBytes(FILE_SYSTEM_ACCESS pFileAccess, UINT32 count, UINT8 *pBuffer);
BOOLEAN fileSystemSkipBytes(FILE_SYSTEM_ACCESS pFileAccess, UINT32 count);
char *fileSystemLoadLine(FILE_SYSTEM_ACCESS pFileAccess, UINT32 count, char *pBuffer);
BOOLEAN fileSystemFileClose(FILE_SYSTEM_ACCESS pFileAccess);

#endif

#endif