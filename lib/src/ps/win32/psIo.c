#include <stdarg.h>

#include "psConfig.h"
#include "psIo.h"





char bannerString[] = "Micro Java O/S\n"
        "Ported to win 32\n"
        "Copyright (C) 2002, MJK corporation, all rights reserved\n";



/* the standard output and error streams */

extern void dumpThreadStates();
void psPutChar(int c)
{
    putchar(c);
    //dumpThreadStates();
}

void psFlushOut()
{
    fflush(stdout);
}

void psPutCharErr(int c)
{
    fputc(c, stderr);
}

void psPrintOut(const char *str)
{
    fputs(str, stdout);
}

void psPrintErr(const char *str)
{
    fputs(str, stderr);
}

void psPrintNErr(const char *str, UINT16 strLength)
{
    UINT16 i;

    for(i=0; i<strLength; i++) {
        psPutCharErr(str[i]);
    }
}

void psPrintBanner(const char *str)
{
    psPrintOut(bannerString);
}

/* logging - note that locking is not necessary, it is done at a higher level */

#if LOGGING

FILE *logFile;

/* returns a non-zero integer if initialization fails */
SUCCESS_CODE initializeLogging() 
{
    logFile = fopen("c:\\projects\\mjvmk\\log.txt", "w");
    return (logFile == NULL) ? SC_FAILURE : SC_SUCCESS;
}

void logFlush()
{
    fflush(logFile);
}

void logNewLine()
{
    fprintf(logFile, "\n");
}

void logChar(int c)
{
    fputc(c, logFile);
}

void logPrintf(const char *format, ...)
{
    va_list argList;

    va_start(argList, format);
    vfprintf(logFile, format, argList);
    va_end(argList);
    logFlush();
}

#endif


#if USE_FILE_SYSTEM

/* file system access */



BOOLEAN psFileSystemFileOpen(const char *path, void (*handleReadError) (FILE_SYSTEM_ACCESS pFileAccess), FILE_SYSTEM_ACCESS pFileAccess)
{
    FILE *fp = fopen(path, "rb");
    if(fp == NULL) {
        return FALSE;
    }
    pFileAccess->fp = fp;
    pFileAccess->handleReadError = handleReadError;
    pFileAccess->hasCachedByte = FALSE;
    return TRUE;
}



BOOLEAN fileSystemEof(FILE_SYSTEM_ACCESS pFileAccess)
{
    int byteRead;

    if(feof(pFileAccess->fp)) {
        return TRUE;
    }
    byteRead = getc(pFileAccess->fp);
    if (byteRead == EOF) {
        return TRUE;
    }
    pFileAccess->cachedByte = byteRead;
    pFileAccess->hasCachedByte = TRUE;
    return FALSE;
}

UINT8 fileSystemLoadByte(FILE_SYSTEM_ACCESS pFileAccess)
{
    register int c;
    
    if(pFileAccess->hasCachedByte) {
        pFileAccess->hasCachedByte = FALSE;
        c = pFileAccess->cachedByte;
    }
    else {
        c = getc(pFileAccess->fp);
        if(c == EOF) {
            pFileAccess->handleReadError(pFileAccess);
            return EOF_VALUE;
        }
    }
    return c;
}

UINT16 fileSystemLoadU2(FILE_SYSTEM_ACCESS pFileAccess)
{
    FILE *fp = pFileAccess->fp;
    register UINT16 value;
    register int lastValue;

    if(pFileAccess->hasCachedByte) {
        pFileAccess->hasCachedByte = FALSE;
        value = pFileAccess->cachedByte << 8;
    }
    else {
        value = getc(fp) << 8;
    }
    lastValue = getc(fp);
    if(lastValue == EOF) {
        pFileAccess->handleReadError(pFileAccess);
        return EOF_VALUE;
    }
    return value | lastValue;
}

UINT32 fileSystemLoadU4(FILE_SYSTEM_ACCESS pFileAccess)
{
    FILE *fp = pFileAccess->fp;
    register UINT32 value;
    register int lastValue;
    
    if(pFileAccess->hasCachedByte) {
        pFileAccess->hasCachedByte = FALSE;
        value = ((((((UINT32) pFileAccess->cachedByte) << 8) | (UINT32) getc(fp)) << 8) | (UINT32) getc(fp)) << 8;
    }
    else {
        value = ((((((UINT32) getc(fp)) << 8) | (UINT32) getc(fp)) << 8) | (UINT32) getc(fp)) << 8;
    }
    lastValue = getc(fp);
    if(lastValue == EOF) {
        pFileAccess->handleReadError(pFileAccess);
        return EOF_VALUE;
    }
    return value | lastValue;
}

UINT32 fileSystemLoadBytes(FILE_SYSTEM_ACCESS pFileAccess, UINT32 count, UINT8 *pBuffer)
{
    register UINT32 numRead;
    
    if(pFileAccess->hasCachedByte) {
        if(count == 0) {
            return 0;
        }
        *pBuffer = pFileAccess->cachedByte;
        pFileAccess->hasCachedByte = FALSE;
        numRead = fread(pBuffer + 1, sizeof(UINT8), count - 1, pFileAccess->fp) + 1;
    }
    else {
        numRead = fread(pBuffer, sizeof(UINT8), count, pFileAccess->fp);
    }

    if(numRead < count) {
        pFileAccess->handleReadError(pFileAccess);
        return EOF_VALUE;
    }
    return numRead;
}

BOOLEAN fileSystemSkipBytes(FILE_SYSTEM_ACCESS pFileAccess, UINT32 count)
{
    register int ret;
    
    if(pFileAccess->hasCachedByte) {
        if(count == 0) {
            return TRUE;
        }
        pFileAccess->hasCachedByte = FALSE;
        ret = fseek(pFileAccess->fp, count - 1, SEEK_CUR);
    }
    else {
        ret = fseek(pFileAccess->fp, count, SEEK_CUR);
    }
    if(ret != 0) {
        pFileAccess->handleReadError(pFileAccess);
        return FALSE;
    }
    return TRUE;
}

char *fileSystemLoadLine(FILE_SYSTEM_ACCESS pFileAccess, UINT32 count, char *pBuffer)
{
    if(pFileAccess->hasCachedByte) {
        if(count == 0) {
            return pBuffer;
        }
        *pBuffer = pFileAccess->cachedByte;
        pFileAccess->hasCachedByte = FALSE;
        fgets(pBuffer + 1, count - 1, pFileAccess->fp);
        return pBuffer;
    }
    return fgets(pBuffer, count, pFileAccess->fp);
}

BOOLEAN fileSystemFileClose(FILE_SYSTEM_ACCESS pFileAccess)
{
    int ret;
    
    pFileAccess->hasCachedByte = FALSE;
    ret = fclose(pFileAccess->fp);
    if(ret != 0) {
        pFileAccess->handleReadError(pFileAccess);
        return FALSE;
    }
    return TRUE;
}

#endif

