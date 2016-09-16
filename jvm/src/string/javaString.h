#ifndef JAVASTRING_H
#define JAVASTRING_H

#include "elements/base.h"
#include "interpret/returnCodes.h"

BOOLEAN verifyUTF8String(char *string, UINT16 length);
jchar getUnicodeCharFromUTF8Char(const char *utf8String, UINT8 *charLength);
UINT16 getUnicodeLength(const char *utf8String, UINT16 utf8Length);

void getUTF8StringFromStringInstance(STRING_INSTANCE pObject, char *utf8String);

RETURN_CODE instantiateCharacterArray(const char *utf8String, UINT16 utf8Length, BOOLEAN permanent, CHAR_ARRAY_INSTANCE *ppInstance);

RETURN_CODE instantiateString(const char *stringArg, UINT16 utflength, OBJECT_INSTANCE* ppString);

RETURN_CODE instantiateInternedString(const char *stringArg, UINT16 utflength, OBJECT_INSTANCE* ppString);


#endif