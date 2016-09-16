#ifndef UNLINKEDSTATICCLASSFILES_H
#define UNLINKEDSTATICCLASSFILES_H

#include "elements/javaParameters.h"
#include "resolution/file/memClassFile.h"

#if USE_UNLINKED_STATIC_CLASS_FILES

extern MEM_FILE unlinkedStaticClassFiles[];
extern UINT16 numUnlinkedStaticFiles;

#define getUnlinkedStaticClassFileEntry() getMemClassPathEntry(unlinkedStaticClassFiles, numUnlinkedStaticFiles)

#endif

#endif



