
#include <stdio.h>
#include <stdlib.h>
#include "psTime.h"
#include "psIo.h"
#include "psConfig.h"

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#include <windows.h>


void psSleep()
{
    /* for power conservation, this function will put the device into sleep mode */
    /* could disable the tick interrupt if so desired and reenable after the sleep mode exits */
    /* if no such mode exists, then you may put a timeout here or even nothing at all */

    Sleep(0);
}

SUCCESS_CODE psInitializeHeap(void **pHeap, size_t heapSize)
{
    *pHeap = malloc(heapSize);
    if(pHeap) {
        return SC_SUCCESS;
    }
    return SC_FAILURE;
}

SUCCESS_CODE psInitializeCPU()
{
    return SC_SUCCESS;
}

#if TESTING

void assertFailure(char *str, char *file, int line, char *compilationDate, char *compilationTime) 
{
    fprintf(stderr, "Assert failure: %s\n%s line %d\ncompiled %s %s\n", 
            str, file, line, compilationDate, compilationTime);
    exit(-1);
}

#endif










