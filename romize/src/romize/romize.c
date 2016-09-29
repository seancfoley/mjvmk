
#include <stdio.h>
#include <string.h>


#include "elements/base.h"
#include "memory/heap.h"
#include "log/logItem.h"
#include "resolution/file/memClassFile.h"
#include "romize.h"

#define MAX_FILE_NAME_LEN 60
#define BYTES_PER_LINE 15


BOOLEAN integrate;

void finalizeSourceFile(FILE *pOutputSourceFile, UINT16 numFiles)
{
    UINT16 i;
    char *FILE_NUM_TAG;
    char *FILE_COLLECTION_TAG;
    char *FILE_TAG;

    if(integrate) { //TODO standardize this if possible
        FILE_NUM_TAG = "numStaticFiles";
        FILE_COLLECTION_TAG = "staticClassFiles";
        FILE_TAG = "memFile";
    }
    else {
        FILE_NUM_TAG = "numUnlinkedStaticFiles";
        FILE_COLLECTION_TAG = "unlinkedStaticClassFiles";
        FILE_TAG = "dynamicMemFile";
    }

    fprintf(pOutputSourceFile, "MEM_FILE %s[] = {\n", FILE_COLLECTION_TAG);
    for(i=0; i<numFiles - 1; i++) {
        fprintf(pOutputSourceFile, "    &%s%d,\n", FILE_TAG, i);
    }
    fprintf(pOutputSourceFile, "    &%s%d\n};\n\n", FILE_TAG, i);
    fprintf(pOutputSourceFile, "UINT16 %s = %d;\n\n", FILE_NUM_TAG, numFiles);
    if(integrate) {
        fprintf(pOutputSourceFile, "#endif");
    }
    fprintf(pOutputSourceFile, "\n\n");
}

void romizeClass(char *name, UINT16 nameLen, FILE_ACCESS pFile, FILE *pOutputFile, UINT16 index)
{
    UINT16 bytePerLineCounter = 0;
    UINT16 byteCounter = 0;
    UINT8 byte;
    char *FILE_TAG;

    if(integrate) { //TODO standardize this if possible
        FILE_TAG = "memFile";
    }
    else {
        FILE_TAG = "dynamicMemFile";    
    }

    /* start the class file entry */
    fprintf(pOutputFile, "byte %s%dBytes[] = {\n", FILE_TAG, index);

    while(!pFile->pFileAccessMethods->eof(pFile)) {
        byte = pFile->pFileAccessMethods->loadByte(pFile);
        bytePerLineCounter++;
        if(bytePerLineCounter > BYTES_PER_LINE) {
            fprintf(pOutputFile, ", \n0x%.2x", byte);
            bytePerLineCounter = 1;
        }
        else if(byteCounter == 0) {
            fprintf(pOutputFile, "0x%.2x", byte);
        }
        else {
            fprintf(pOutputFile, ", 0x%.2x", byte);
        }
        byteCounter++;
    }

    /* finish off the class file entry */
    fprintf(pOutputFile, "\n};\n\n");
    
    /* print the memFileStruct structure */
    fprintf(pOutputFile, "memFileStruct %s%d = {\n    \"%s\", %d, %s%dBytes, %d\n};\n\n",
        FILE_TAG, index, name, nameLen, FILE_TAG, index, byteCounter);

}

void usage()
{
    printf("usage: encode listing-file output-file.c [-classpath dir1;dir2;...] [-integrate]\n");
}
        
int main(int argc, char *argv[])
{
    RETURN_CODE ret;
    FILE_ACCESS pFileAccess;
    UINT16 i = 0;
    FILE *pInputTextFile;
    FILE *pOutputSourceFile;
    UINT16 fileNameLen;
	char fileNameBuffer[MAX_FILE_NAME_LEN];
    char *classpath;
    int currentArg;
    
    if(argc < 3 || argc > 6) {
        usage();
        return -1;
    }
    
    

    if(argc > 3) {
        currentArg = 3;
        while(argc > currentArg) {
            if(strcmp(argv[currentArg], "-classpath") == 0) {
                if(argc == currentArg + 1) {
                    usage();
                    return -1;
                }
                classpath = argv[currentArg + 1];
                currentArg += 2;
                continue;
            }
            else if(strcmp(argv[currentArg], "-integrate") == 0 ) {
                integrate = TRUE;
                currentArg++;
                continue;
            }
            else {
                usage();
                return -1;
            }
        }
    }
    else {
        classpath = ".";
        integrate = FALSE;
    }

    pInputTextFile = fopen(argv[1], "r");
    if(pInputTextFile == NULL) {
        LOG_LINE(("failed to open input file %s", argv[1]));
        return -1;
    }

    pOutputSourceFile = fopen(argv[2], "w");
    if(pOutputSourceFile == NULL) {
        LOG_LINE(("failed to open output file %s", argv[2]));
        fclose(pInputTextFile);
        return -1;
    }

    ret = initializeHeap();
    if(ret != SUCCESS) {
        LOG_LINE(("failed initialize heap"));
        return -1;
    }

    ret = initializeFileAccess(classpath);
    if(ret != SUCCESS) {
        LOG_LINE(("failed initialize class file access with class path %s", classpath));
        return -1;
    }
    

    /* create the source file */
    
    
    if(integrate) {
        fprintf(pOutputSourceFile, "#include \"staticClassFiles.h\"\n\n"
            "#if USE_STATIC_CLASS_FILES\n\n");
    }
    else {
        fprintf(pOutputSourceFile, "\ntypedef unsigned char byte;\ntypedef unsigned int UINT16;\n"
            "\ntypedef struct memFileStruct {\n"
            "   char *name;\n"
            "   UINT16 nameLength;\n"
            "   byte *pData;\n"
            "   UINT16 dataLength;\n"
            "} memFileStruct, *MEM_FILE;\n\n"
            );
    }

    while(TRUE) {
        if(fgets(fileNameBuffer, MAX_FILE_NAME_LEN, pInputTextFile) == NULL) {
            break;
        }
        fileNameLen = (UINT16) strlen(fileNameBuffer);
        while(fileNameLen > 0 && fileNameBuffer[fileNameLen - 1] == '\n' || fileNameBuffer[fileNameLen - 1] == '\r') {
            fileNameBuffer[fileNameLen - 1] = '\0';
            fileNameLen--;
        }
        if(fileNameLen == 0) {
            continue;
        }

        pFileAccess = openFile(fileNameBuffer, fileNameLen);
        if(pFileAccess == NULL) {
            LOG_LINE(("failed to find file %s", fileNameBuffer));
            fprintf(stderr, "failed to find file %s\n", fileNameBuffer);
            continue;
        }

        romizeClass(fileNameBuffer, fileNameLen, pFileAccess, pOutputSourceFile, i);
        pFileAccess->pFileAccessMethods->close(pFileAccess);
        i++;
    }
    finalizeSourceFile(pOutputSourceFile, i);
    fclose(pOutputSourceFile);
    finalizeFileAccess();
    return 0;
}


//TODO: romizer convert class files into C source files with
//loadedClassDefStruct's which are fully constant-pool-resolved quickened.  They are stored
//in a special romized class table with no pointers anywhere which is coped directly to the new class table of every process.


