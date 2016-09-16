
#include "elements/javaParameters.h"
#include "memory/memoryAlloc.h"
#include "fileSystemClassFile.h"
#include "fileAccess.h"

#if USE_STATIC_CLASS_FILES
#include "staticClassFiles.h"
#endif

#if USE_UNLINKED_STATIC_CLASS_FILES
#include "unlinkedStaticClassFiles.h"
#endif


typedef struct classPathStruct {
    UINT16 length;
    CLASS_PATH_ENTRY entries[1];  
} classPathStruct, *CLASS_PATH;

#define getClassPathStructSize(numEntries) (structOffsetOf(classPathStruct, entries) + (numEntries) * sizeof(CLASS_PATH_ENTRY))

static CLASS_PATH pClassPath = NULL;

FILE_ACCESS openFileInClassPathEntry(char *fileName, UINT16 fileNameLength, UINT16 classPathIndex)
{
    return pClassPath->entries[classPathIndex]->getFileAccess(pClassPath->entries[classPathIndex], fileName, fileNameLength);
}

FILE_ACCESS openFile(char *fileName, UINT16 fileNameLength)
{
    UINT16 i;
    FILE_ACCESS pFileAccess = NULL;
    
    for(i=0; i < pClassPath->length; i++) {
        pFileAccess = openFileInClassPathEntry(fileName, fileNameLength, i);
        if(pFileAccess != NULL) {
            pFileAccess->classPathIndex = i;
            break;
        }
    }
    return pFileAccess;
}



/* fileName will have '/' as the separator for the full class name with package */
RETURN_CODE openClassFile(const char *packageName, UINT16 packageNameLength, const char *className, UINT16 classNameLength, FILE_ACCESS *ppFileAccess) 
{
    char *fileName;
    FILE_ACCESS pFileAccess;
    UINT16 fileNameLength = packageNameLength + classNameLength + 7;
    
    fileName = memoryAlloc(fileNameLength);
    if(fileName == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    memoryCopy(fileName, packageName, packageNameLength);
    fileName[packageNameLength] = '/';
    memoryCopy(fileName + packageNameLength + 1, className, classNameLength);
    memoryCopy(fileName + packageNameLength + classNameLength + 1, ".class", 6);
    
    pFileAccess = openFile(fileName, fileNameLength);
    
    memoryFree(fileName);

    *ppFileAccess = pFileAccess;
    if(pFileAccess == NULL) {
        return ERROR_CODE_NO_CLASS_DEF_FOUND;
    }
    return pFileAccess->errorFlag;
}

RETURN_CODE initializeFileAccess(const char *classPathString) 
{
    UINT16 classPathStringLength = stringLength(classPathString);
    UINT16 numEntries = 0;
#if USE_UNLINKED_STATIC_CLASS_FILES
    CLASS_PATH_ENTRY pUnlinkedClassPathEntry;
#endif

#if USE_STATIC_CLASS_FILES
    CLASS_PATH_ENTRY pStaticClassPathEntry = getStaticClassFileEntry();

    if(pStaticClassPathEntry) {
        numEntries++;
    }
#endif

#if USE_UNLINKED_STATIC_CLASS_FILES
    pUnlinkedClassPathEntry = getUnlinkedStaticClassFileEntry();

    if(pUnlinkedClassPathEntry) {
        numEntries++;
    }
#endif

#if USE_STATIC_CLASS_FILES || USE_UNLINKED_STATIC_CLASS_FILES || USE_FILE_SYSTEM_CLASS_FILES

    if(classPathString != NULL && *classPathString) {
        UINT16 i;
        numEntries++;
        for(i=0; i < classPathStringLength; i++) {
            if(classPathString[i] == CLASS_PATH_SEPARATOR) {
                numEntries++;
            }
        }
    }

#endif

    pClassPath = memoryAlloc(getClassPathStructSize(numEntries));
    if(pClassPath == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    pClassPath->length = 0;
        
#if USE_STATIC_CLASS_FILES
    if(pStaticClassPathEntry) {
        pClassPath->entries[pClassPath->length] = pStaticClassPathEntry;
        pClassPath->length++;
    }
#endif
#if USE_UNLINKED_STATIC_CLASS_FILES
    if(pUnlinkedClassPathEntry) {
        pClassPath->entries[pClassPath->length] = pUnlinkedClassPathEntry;
        pClassPath->length++;
    }
#endif
#if USE_FILE_SYSTEM_CLASS_FILES
    if(classPathStringLength > 0) {
        UINT16 i;
        const char *cp;
        
        for(cp=classPathString, i=0; ; i++) {
    
            if(classPathString[i] == CLASS_PATH_SEPARATOR || i == classPathStringLength) {
           
                CLASS_PATH_ENTRY pClassPathEntry = getFileSystemClassPathEntry(cp, (UINT16) ((classPathString + i) - cp)); 
                if(pClassPathEntry == NULL) {
                    finalizeFileAccess();
                    return ERROR_CODE_OUT_OF_MEMORY;
                }

                pClassPath->entries[pClassPath->length] = pClassPathEntry;
                pClassPath->length++;

                if(i == classPathStringLength) {
                    break;
                }
                cp = classPathString + i + 1;
            }
        }
    }
#endif

    return SUCCESS;
}

void finalizeFileAccess() 
{
    UINT16 i;

    for(i=0; i<pClassPath->length; i++) {
        memoryFree(pClassPath->entries[i]);
    }

    memoryFree(pClassPath);
}



