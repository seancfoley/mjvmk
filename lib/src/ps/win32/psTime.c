
#include "thread/contextSwitch.h"
#include "psTypes.h"
#include "psTime.h"

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

pstime psGetCurrentTimeInMillis()
{
    FILETIME fileTime;
    
    GetSystemTimeAsFileTime(&fileTime);

    /* units are 100 nanoseconds, time is since Jan. 1, 1601 */
    /* The windows system time of Jan 1, 1970 00:00:00 UTC is 11644473600000 after conversion to milliseconds */
    
    return (((pstime) fileTime.dwHighDateTime << 32 | (pstime) fileTime.dwLowDateTime) / 10000) - 11644473600000ui64;
}

