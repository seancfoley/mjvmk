#ifndef STATICCLASSFILES_H
#define STATICCLASSFILES_H

#include "elements/javaParameters.h"
#include "resolution/file/memClassFile.h"

#if USE_STATIC_CLASS_FILES

extern MEM_FILE staticClassFiles[];
extern UINT16 numStaticFiles;

#define getStaticClassFileEntry() getMemClassPathEntry(staticClassFiles, numStaticFiles)

#endif

#endif

