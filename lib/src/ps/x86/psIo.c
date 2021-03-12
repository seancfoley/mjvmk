#include <stdarg.h>

#include "psConfig.h"
#include "psTime.h"
#include "psIo.h"



char bannerString[] = "Micro Java Kernel\n"
        "Ported to x86 in protected mode\n"
        "Copyright (C) 2002, MJK corporation, all rights reserved\n";


/* logging - note that locking is not necessary, it is done at a higher level */

/* the standard output and error streams */

#define DISPLAY_RAM_BASE ((unsigned char *) 0xb8000)
#define NUM_DISPLAY_COLUMNS 80
#define NUM_DISPLAY_ROWS 25
#define BLACK_ON_WHITE_ATTRIBUTE 0x07

int currentCharXCoordinate = 0;
int currentCharYCoordinate = 0;

void psPutNewLineScreen()
{
    unsigned char *pScreen;
    int i;

    /* we need to move up all previously displayed lines */
    memoryCopy(DISPLAY_RAM_BASE, DISPLAY_RAM_BASE + NUM_DISPLAY_COLUMNS * 2, 
        (NUM_DISPLAY_ROWS - 1) * NUM_DISPLAY_COLUMNS * 2);
    pScreen = DISPLAY_RAM_BASE + (NUM_DISPLAY_ROWS - 1) * NUM_DISPLAY_COLUMNS * 2;
    for(i=0; i<NUM_DISPLAY_COLUMNS; i++) {
        *pScreen++ = ' ';
        *pScreen++ = BLACK_ON_WHITE_ATTRIBUTE;
    }
}

void psPutCharScreen(int c)
{
    unsigned char *pScreen;

    enterCritical();
    
    
    switch(c) {
        case '\r':
            break;
        case '\n':
            if(currentCharYCoordinate == NUM_DISPLAY_ROWS - 1) {
                psPutNewLineScreen();
            }
            else {
                currentCharYCoordinate++;
            }
            currentCharXCoordinate = 0;
            break;
        default:
            if(currentCharXCoordinate == NUM_DISPLAY_COLUMNS) {
                if(currentCharYCoordinate == NUM_DISPLAY_ROWS - 1) {
                    psPutNewLineScreen();
                }
                else {
                    currentCharYCoordinate++;
                }
                currentCharXCoordinate = 0;
            }
            pScreen = DISPLAY_RAM_BASE + 2 * (currentCharYCoordinate * NUM_DISPLAY_COLUMNS + currentCharXCoordinate);
            *pScreen++ = c;
            *pScreen++ = BLACK_ON_WHITE_ATTRIBUTE;
            currentCharXCoordinate++;
    }

    exitCritical();

}

void psPutCharErr(int c)
{
    psPutCharScreen(c);
}

void psPutChar(int c)
{
    psPutCharScreen(c);
}

void psFlushOut() {}


void psPrintOut(const char *str)
{
    while(*str) {
        psPutCharScreen(*str);
        str++;
    }
}


void psPrintErr(const char *str)
{
    while(*str) {
        psPutCharScreen(*str);
        str++;
    }
}

void psPrintBanner()
{
    unsigned char *pScreen = DISPLAY_RAM_BASE;
    int i;
    int screenSize = NUM_DISPLAY_ROWS * NUM_DISPLAY_COLUMNS;

    /* first clear the screen */
    for(i=0; i<screenSize; i++) {
        *pScreen++ = ' ';
        *pScreen++ = BLACK_ON_WHITE_ATTRIBUTE;
    }

    psPrintOut(bannerString);
}


void psPrintNErr(const char *str, UINT16 strLength)
{
    UINT16 i;

    for(i=0; i<strLength; i++) {
        psPutCharErr(str[i]);
    }
}







#if LOGGING

SUCCESS_CODE initializeLogging() 
{
    return SC_SUCCESS;
}

void logFlush()
{
    return;
}

void logNewLine()
{
    psPutChar('\n');
}

void logChar(int c)
{
    psPutChar(c);
}

char logBuffer[NUM_DISPLAY_COLUMNS + 1];

void logPrintf(const char *format, ...)
{
    va_list argList;

    va_start(argList, format);
    _vsnprintf(logBuffer, NUM_DISPLAY_COLUMNS, format, argList);
    va_end(argList);
    psPrintOut(logBuffer);
    return;
}

#endif

#if USE_FILE_SYSTEM

/* file system access */

BOOLEAN psFileSystemFileOpen(const char *path, void (*handleReadError) (FILE_SYSTEM_ACCESS pFileAccess), FILE_SYSTEM_ACCESS pFileAccess)
{
    return FALSE;
}

BOOLEAN fileSystemEof(FILE_SYSTEM_ACCESS pFileAccess)
{
    return TRUE;
}

UINT8 fileSystemLoadByte(FILE_SYSTEM_ACCESS pFileAccess)
{
    pFileAccess->handleReadError(pFileAccess);
    return EOF_VALUE;
}

UINT16 fileSystemLoadU2(FILE_SYSTEM_ACCESS pFileAccess)
{
    pFileAccess->handleReadError(pFileAccess);
    return EOF_VALUE;
}

UINT32 fileSystemLoadU4(FILE_SYSTEM_ACCESS pFileAccess)
{
    pFileAccess->handleReadError(pFileAccess);
    return EOF_VALUE;
}

UINT32 fileSystemLoadBytes(FILE_SYSTEM_ACCESS pFileAccess, UINT32 count, UINT8 *pBuffer)
{
    pFileAccess->handleReadError(pFileAccess);
    return 0;
}

BOOLEAN fileSystemSkipBytes(FILE_SYSTEM_ACCESS pFileAccess, UINT32 count)
{
    pFileAccess->handleReadError(pFileAccess);
    return FALSE;
}

BOOLEAN fileSystemFileClose(FILE_SYSTEM_ACCESS pFileAccess) 
{
    pFileAccess->handleReadError(pFileAccess);
    return FALSE;
}

char *fileSystemLoadLine(FILE_SYSTEM_ACCESS pFileAccess, UINT32 count, char *pBuffer) 
{
    pFileAccess->handleReadError(pFileAccess);
    return NULL;
}


#endif





