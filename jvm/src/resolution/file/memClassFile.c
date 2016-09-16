
#include "elements/javaParameters.h"
#include "memory/memoryAlloc.h"
#include "memClassFile.h"

#if USE_STATIC_CLASS_FILES || USE_UNLINKED_STATIC_CLASS_FILES


/* mirrors the structure of a fileAccessStruct, represents a file in memory that was included 
 * in the compile-time build.
 */
typedef struct memFileAccessStruct {
    fileAccessStruct genericFileAccess;
    PBYTE pCurrentByte;
    PBYTE eof;
} memFileAccessStruct, *MEM_FILE_ACCESS;

static void handleMemReadError(MEM_FILE_ACCESS pFileAccess)
{
    pFileAccess->genericFileAccess.errorFlag = ERROR_CODE_INVALID_FILE_SIZE; 
    if(pFileAccess->genericFileAccess.pErrorJump) {
        longJump(*(pFileAccess->genericFileAccess.pErrorJump), -1);
    }
}

static UINT8 memLoadByte(MEM_FILE_ACCESS pFileAccess)
{
    register UINT8 val;

    if(pFileAccess->pCurrentByte >= pFileAccess->eof) {
        handleMemReadError(pFileAccess);
        return EOF_VALUE;
    }
    val = pFileAccess->pCurrentByte[0];
    pFileAccess->pCurrentByte++;
    return val;
}

static UINT16 memLoadU2(MEM_FILE_ACCESS pFileAccess)
{
    register UINT16 val;

    if(pFileAccess->pCurrentByte + 1 >= pFileAccess->eof) {
        handleMemReadError(pFileAccess);
        return EOF_VALUE;
    }

#define loadedShortValue(x) ((UINT16) (pFileAccess->pCurrentByte[x]))
    
    val = (loadedShortValue(0) << 8) | loadedShortValue(1);
    pFileAccess->pCurrentByte += 2;
    return val;
}

static UINT32 memLoadU4(MEM_FILE_ACCESS pFileAccess)
{
    register UINT32 val;
    
    if(pFileAccess->pCurrentByte + 3 >= pFileAccess->eof) {
        handleMemReadError(pFileAccess);
        return EOF_VALUE;
    }

#define loadedIntValue(x) ((UINT32) (pFileAccess->pCurrentByte[x]))
    
    val = (((((loadedIntValue(0) << 8) 
        | loadedIntValue(1)) << 8)
        | loadedIntValue(2)) << 8)
        | loadedIntValue(3);

    pFileAccess->pCurrentByte += 4;
    return val;
}

static UINT32 memLoadBytes(MEM_FILE_ACCESS pFileAccess, UINT32 count, UINT8 *pBuffer)
{
    if(pFileAccess->pCurrentByte + count > pFileAccess->eof) {
        handleMemReadError(pFileAccess);
        return EOF_VALUE;
    }
    memoryCopy(pBuffer, pFileAccess->pCurrentByte, count);
    pFileAccess->pCurrentByte += count;
    return count;
}

static BOOLEAN memSkipBytes(MEM_FILE_ACCESS pFileAccess, UINT32 count)
{
    if(pFileAccess->pCurrentByte + count > pFileAccess->eof) {
        handleMemReadError(pFileAccess);
        return FALSE;
    }
    pFileAccess->pCurrentByte += count;
    return TRUE;
}

static char *memLoadLine(MEM_FILE_ACCESS pFileAccess, UINT32 count, unsigned char *pBuffer)
{
    unsigned char *pBufferOriginal = pBuffer;
    unsigned char c;

    while(TRUE) {
        if(pFileAccess->pCurrentByte == pFileAccess->eof) {
            *pBuffer = '\0';
            return NULL;
        }
        if(--count == 0) {
            *pBuffer = '\0';
            return pBufferOriginal;
        }
        *pBuffer = c = (unsigned char) (*pFileAccess->pCurrentByte);
        pFileAccess->pCurrentByte++;
        if(c == '\n') {
            pBuffer[1] = '\0';
            return pBufferOriginal;
        }
        pBuffer++;
    }
}

static BOOLEAN memClose(MEM_FILE_ACCESS pFileAccess)
{
    memoryFree(pFileAccess);
    return TRUE;
}

static BOOLEAN memEof(MEM_FILE_ACCESS pFileAccess)
{
    return (pFileAccess->pCurrentByte >= pFileAccess->eof);
}

fileAccessMethodsStruct memFileAccessMethods = {
    (UINT8 (*) (FILE_ACCESS)) memLoadByte,
    (UINT16 (*) (FILE_ACCESS)) memLoadU2,
    (UINT32 (*) (FILE_ACCESS)) memLoadU4,
    (UINT32 (*) (FILE_ACCESS, UINT32, UINT8 *)) memLoadBytes,
    (char *(*) (FILE_ACCESS, UINT32, char *)) memLoadLine,
    (BOOLEAN (*) (FILE_ACCESS, UINT32)) memSkipBytes,
    (BOOLEAN (*) (FILE_ACCESS)) memClose,
    (BOOLEAN (*) (FILE_ACCESS)) memEof       
};

static FILE_ACCESS getOutOfMemoryMemFileAccess() 
{
    static memFileAccessStruct memFileAccess;
    
    memFileAccess.pCurrentByte = memFileAccess.eof = NULL;
    memFileAccess.genericFileAccess.errorFlag = ERROR_CODE_OUT_OF_MEMORY;
    return (FILE_ACCESS) &memFileAccess;
}

/* a memory class path entry consists of an array of byte arrays, 
 * each byte array representing a class file.
 */
typedef struct memClassPathEntryStruct {
    classPathEntryStruct genericEntry;
    UINT16 numFiles;
    MEM_FILE *classFiles;
} memClassPathEntryStruct, *MEM_CLASS_PATH_ENTRY;

#define getMemFileAccessStructSize() (sizeof(memFileAccessStruct))



FILE_ACCESS memFileOpen(CLASS_PATH_ENTRY pClassPathEntry, const char *fileName, UINT16 fileNameLength) 
{
    UINT16 i;
    MEM_FILE pCurrentFile;
    MEM_CLASS_PATH_ENTRY pEntry = (MEM_CLASS_PATH_ENTRY) pClassPathEntry;
    
    for(i=0; i<pEntry->numFiles; i++) {
        pCurrentFile = pEntry->classFiles[i];
        
        if(fileNameLength == pCurrentFile->nameLength && memoryCompare(fileName, pCurrentFile->name, fileNameLength) == 0) {
            
            /* we've found the required file */
            
            MEM_FILE_ACCESS pMemFileAccess;
            FILE_ACCESS pFileAccess;
            
            pMemFileAccess = memoryAlloc(getMemFileAccessStructSize());
            if(pMemFileAccess == NULL)  {
                return getOutOfMemoryMemFileAccess();
            }
            pMemFileAccess->pCurrentByte = pCurrentFile->pData;
            pMemFileAccess->eof = pCurrentFile->pData + pCurrentFile->dataLength;
            pFileAccess = &pMemFileAccess->genericFileAccess;
            pFileAccess->pFileAccessMethods = &memFileAccessMethods;
            pFileAccess->pErrorJump = NULL;
            pFileAccess->errorFlag = SUCCESS;
            
            return (FILE_ACCESS) pMemFileAccess;
        }
    }
    return NULL;
}


CLASS_PATH_ENTRY getMemClassPathEntry(MEM_FILE classFiles[], UINT16 numFiles)
{
    MEM_CLASS_PATH_ENTRY memEntry;

    if(numFiles == 0) {
        return NULL;
    }
    
    memEntry = memoryAlloc(sizeof(memClassPathEntryStruct));
    if(memEntry != NULL) {
        memEntry->numFiles = numFiles;
        memEntry->classFiles = classFiles;
        memEntry->genericEntry.getFileAccess = memFileOpen;
    }
    return (CLASS_PATH_ENTRY) memEntry;
}

#endif
