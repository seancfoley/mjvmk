#ifndef FILE_ACCESS_H
#define FILE_ACCESS_H

#include "psIo.h"
#include "types/config.h"
#include "interpret/returnCodes.h"






typedef struct classPathEntryStruct classPathEntryStruct, *CLASS_PATH_ENTRY;

struct classPathEntryStruct {
    /* given a class name, this function returns the structure used to access that class, if it is available from this entry */
    FILE_ACCESS (*getFileAccess) (CLASS_PATH_ENTRY, const char *fileName, UINT16 fileNameLength);
};

FILE_ACCESS openFile(char *fileName, UINT16 fileNameLength);
FILE_ACCESS openFileInClassPathEntry(char *fileName, UINT16 fileNameLength, UINT16 classPathIndex);
RETURN_CODE openClassFile(const char *packageName, UINT16 packageNameLength, const char *className, UINT16 classNameLength, FILE_ACCESS *ppFileAccess);

RETURN_CODE initializeFileAccess(const char *classPathString);
void finalizeFileAccess();

/* all the functions in this structure take a pointer to the file access structure corresponding
 * to a particular file, whether in a file system, a jar file, or static memory.
 */
struct fileAccessMethodsStruct {
    UINT8 (*loadByte) (FILE_ACCESS pFileAccess);
    UINT16 (*loadU2) (FILE_ACCESS pFileAccess);
    UINT32 (*loadU4) (FILE_ACCESS pFileAccess);
    UINT32 (*loadBytes) (FILE_ACCESS pFileAccess, UINT32 count, UINT8 *buffer);
    char *(*loadLine) (FILE_ACCESS pFileAccess, UINT32 count, char *buffer);
    BOOLEAN (*skipBytes) (FILE_ACCESS pFileAccess, UINT32 count);
    BOOLEAN (*close) (FILE_ACCESS pFileAccess);
    BOOLEAN (*eof) (FILE_ACCESS pFileAccess);
};


#endif
