#ifndef MEMCLASSFILE_H
#define MEMCLASSFILE_H


#include "fileAccess.h"




/* the romizer will create one of these for each class file, and pass an array of these structs to get
 * the class path entry
 */
typedef struct memFileStruct {
    char *name;
    UINT16 nameLength;
    PBYTE pData;
    UINT16 dataLength;
} memFileStruct, *MEM_FILE;


CLASS_PATH_ENTRY getMemClassPathEntry(MEM_FILE classFiles[], UINT16 numFiles);


#endif
