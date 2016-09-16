
#include "psIo.h"
#include "memory/memoryAlloc.h"
#include "elements/javaParameters.h"
#include "fileSystemClassFile.h"

#if USE_FILE_SYSTEM_CLASS_FILES

static void handleReadError(FILE_SYSTEM_ACCESS pFileAccess)
{
    pFileAccess->genericFileAccess.errorFlag = 
            (*(pFileAccess->genericFileAccess.pFileAccessMethods->eof))((FILE_ACCESS) pFileAccess) ? ERROR_CODE_INVALID_FILE_SIZE : ERROR_CODE_FILE_READ_ERROR; 
    if(pFileAccess->genericFileAccess.pErrorJump) {
        longJump(*(pFileAccess->genericFileAccess.pErrorJump), -1);
    }
}

static BOOLEAN releaseFile(FILE_SYSTEM_ACCESS pFileAccess)
{
    BOOLEAN ret = fileSystemFileClose(pFileAccess);
    memoryFree(pFileAccess);
    return ret;
}


fileAccessMethodsStruct fileSystemFileAccessMethods = {
    (UINT8 (*) (FILE_ACCESS)) fileSystemLoadByte,
    (UINT16 (*) (FILE_ACCESS)) fileSystemLoadU2,
    (UINT32 (*) (FILE_ACCESS)) fileSystemLoadU4,
    (UINT32 (*) (FILE_ACCESS, UINT32, UINT8 *)) fileSystemLoadBytes,
    (char *(*) (FILE_ACCESS, UINT32, char *)) fileSystemLoadLine,
    (BOOLEAN (*) (FILE_ACCESS, UINT32)) fileSystemSkipBytes,
    (BOOLEAN (*) (FILE_ACCESS)) releaseFile,
    (BOOLEAN (*) (FILE_ACCESS)) fileSystemEof  
};


static FILE_ACCESS getOutOfMemoryFileSystemAccess() 
{
    static fileSystemAccessStruct errorFileAccess;
    errorFileAccess.genericFileAccess.errorFlag = ERROR_CODE_OUT_OF_MEMORY;
    return (FILE_ACCESS) &errorFileAccess;
}

typedef struct dirClassPathEntryStruct {
    classPathEntryStruct genericEntry;
    char *directory;
    UINT16 directoryStringLength;
} dirClassPathEntryStruct, *DIR_CLASS_PATH_ENTRY;


static FILE_ACCESS fileSystemFileOpen(CLASS_PATH_ENTRY pClassPathEntry, const char *fileName, UINT16 fileNameLength) 
{
    char *fullPath;
    UINT8 sepSize = sizeof(FILE_PATH_SEPARATOR);
    DIR_CLASS_PATH_ENTRY pDirClassPathEntry = (DIR_CLASS_PATH_ENTRY) pClassPathEntry;
    FILE_SYSTEM_ACCESS pFileSystemAccess;
    BOOLEAN successfullyOpened;
    fileSystemAccessStruct tempStruct;

    fullPath = memoryAlloc(pDirClassPathEntry->directoryStringLength + fileNameLength + 1 + sepSize);
    if(fullPath == NULL)  {
        return getOutOfMemoryFileSystemAccess();
    }

    memoryCopy(fullPath, pDirClassPathEntry->directory, pDirClassPathEntry->directoryStringLength);
    memoryCopy(fullPath + pDirClassPathEntry->directoryStringLength, FILE_PATH_SEPARATOR, sepSize - 1);
    memoryCopy(fullPath + pDirClassPathEntry->directoryStringLength + sepSize - 1, fileName, fileNameLength);
    fullPath[pDirClassPathEntry->directoryStringLength + fileNameLength + sepSize - 1] = '\0';
    
    successfullyOpened = psFileSystemFileOpen(fullPath, handleReadError, &tempStruct);
    memoryFree(fullPath);

    if(successfullyOpened) {
        FILE_ACCESS pFileAccess;

        pFileSystemAccess = memoryAlloc(getFileSystemAccessStructSize());
        if(pFileSystemAccess == NULL)  {
            return getOutOfMemoryFileSystemAccess();
        }
        memoryCopy(pFileSystemAccess, &tempStruct, sizeof(fileSystemAccessStruct));
        pFileAccess = &pFileSystemAccess->genericFileAccess;
        pFileAccess->errorFlag = SUCCESS;
        pFileAccess->pErrorJump = NULL;
        pFileAccess->pFileAccessMethods = &fileSystemFileAccessMethods;
        return (FILE_ACCESS) pFileSystemAccess;
    }
    return NULL;
}

CLASS_PATH_ENTRY getFileSystemClassPathEntry(const char *dirString, UINT16 stringLength)
{
    DIR_CLASS_PATH_ENTRY dirEntry;
    char *dir;

    dirEntry = memoryAlloc(sizeof(dirClassPathEntryStruct) + stringLength);
    if(dirEntry != NULL) {
        dir = (char *) (dirEntry + 1);
        memoryCopy(dir, dirString, stringLength);
        dirEntry->directory = dir;
        dirEntry->directoryStringLength = stringLength;
        dirEntry->genericEntry.getFileAccess = fileSystemFileOpen;
    }
    return (CLASS_PATH_ENTRY) dirEntry;
}

#endif
