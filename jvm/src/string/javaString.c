
#include "interpret/javaGlobals.h"
#include "object/instantiate.h"
#include "javaString.h"

/* Routines for string manipulation and instantiation:
 *
 * - conversion from UTF8 strings to Unicode and vice-versa
 * - UTF8 string verification
 * - instantiation of character arrays, java unicode string objects and java unicode interned string objects
 *
 */


jchar getUnicodeCharFromUTF8Char(const char *ptr, UINT8 *charLength) 
{
    char c;
    char c2;
    char c3;
    UINT8 length;
    jchar result;

    switch ((c = ptr[0]) >> 4) {

        case 0x0C: 
        case 0x0D: 
            /* character is 110...  10... */
            c2 = ptr[1];
            if ((c2 & 0xC0) == 0x80) {
                result = ((c & 0x1F) << 6) + (c2 & 0x3F);
                length = 2;
            }
            else {
                /* invalid character */
                length = 1;
                result = 0;
            }
            break;

        case 0x0E:
            /* character is 1110... 10... 10... */
            c2 = ptr[1];
            if ((c2 & 0xC0) == 0x80) {
                c3 = ptr[2];
                if ((c3 & 0xC0) == 0x80) {
                    result = ((((c & 0x0f) << 6) + (c2 & 0x3f)) << 6) + (c3 & 0x3f);
                    length = 3;
                } else {
                    /* invalid character */
                    length = 2;
                    result = 0;
                }
            }
            else {
                /* invalid character */
                length = 1;
                result = 0;
            }
            break;
         
        default:
            /* character is either invalid or a single byte */
            length = 1;
            result = c;
            break;
    }
    *charLength = length;
    return result;
}

UINT16 getUnicodeLength(const char *utf8String, UINT16 utf8Length) 
{
    UINT8 charLength;
    UINT16 totalLength = 0;
    const char *end = utf8String + utf8Length;

    while(utf8String < end) {

        switch (utf8String[0] >> 4) {
          
            case 0x0C: 
            case 0x0D: 
                /* charcater is 110...  10... */
                if ((utf8String[1] & 0xC0) == 0x80) {
                    charLength = 2;
                }
                else { 
                    /* invalid character */
                    charLength = 1;
                }
                break;

            case 0x0E:
                /* character is 1110... 10... 10... */
                if ((utf8String[1] & 0xC0) == 0x80) {
                    if ((utf8String[2] & 0xC0) == 0x80) {
                        charLength = 3;
                    } else {
                        /* invalid character */
                        charLength = 2;
                    }
                }
                else {
                    /* invalid character */
                    charLength = 1;
                }
                break;

            default:
                /* if we get here, the character may be valid or invalid, but that is not our concern */
                charLength = 1;
                break;

        }
        utf8String += charLength;
        totalLength++;
    }
    return totalLength;
}

BOOLEAN verifyUTF8String(char *string, UINT16 length)
{
    unsigned char c;
    unsigned char *end;

    for (end = string + length; string < end; string++) {
        c = *string;
        if (c == 0) {
            /* UTF8 strings are not meant to have terminating NULL characters */
            return FALSE;
        }

        switch (c >> 4) {
            case 0x08: 
            case 0x09: 
            case 0x0A: 
            case 0x0B: 
            case 0x0F:
                return FALSE;
            
            case 0x0E:
                /* character is 1110... 10... 10... */
                string += 2;
                if (string >= end || (*((UINT16 *) (string - 1)) & 0xC0C0) != 0x8080) {
                    return FALSE;
                }
                break;
            
            case 0x0C:
            case 0x0D:
                /* character is 110...  10... */
                string++;
                if (string >= end || (*string & 0xC0) != 0x80) {
                    return FALSE;
                }
                break;
            
            default:
                /* all ascii characters will land here */
                break;
        }
    }
    return TRUE;
}


void getUTF8StringFromStringInstance(STRING_INSTANCE pObject, char *utf8String)
{
    jint i;
    jint stringOffset = extractPrimitiveFieldInt(pObject->offset);
    jint length = extractPrimitiveFieldInt(pObject->length);
    jint charArrayLength = length + stringOffset;
    CHAR_ARRAY_INSTANCE pChars = (CHAR_ARRAY_INSTANCE) extractObjectFieldObject(pObject->pCharArray);

    /* as you can see, only ascii unicode characters are converted correctly by this function. 
     * In the long run this can be expanded to handle as many unicode characters as desired.
     */
    for(i = stringOffset; i < charArrayLength; i++) {
        utf8String[i] = (char) getCharArrayElement(pChars, i);
    }
    return;
}

RETURN_CODE instantiateCharacterArray(const char *utf8String, UINT16 utf8Length, 
                                      BOOLEAN permanent, CHAR_ARRAY_INSTANCE *ppInstance)
{
    RETURN_CODE ret;
    UINT16 i, unicodeLength = 0;
    UINT8 charLength;
    
    unicodeLength = getUnicodeLength(utf8String, utf8Length);

    if((ret = instantiateArrayGeneric(pCharArrayClass, pCharArrayClassDef, 
        sizeof(jchar), unicodeLength, permanent, FALSE, (ARRAY_INSTANCE *) ppInstance)) != SUCCESS) {
        return ret;
    }

    /*  Initialize the array with string characters */
    for (i = 0; i < unicodeLength; i++) { 
        (*ppInstance)->values[i] = getUnicodeCharFromUTF8Char(utf8String, &charLength);
        utf8String += charLength;
    }
    return SUCCESS;
}

RETURN_CODE instantiateString(const char *stringArg, UINT16 utf8Length, OBJECT_INSTANCE *ppResult)
{
    RETURN_CODE ret;
    STRING_INSTANCE pInstance;
    
    if(SUCCESS != (ret = instantiateCollectibleObject(pJavaLangString, ppResult))) {
        return ret;
    }
    pInstance = getStringInstanceFromObjectInstance(*ppResult);
    if(SUCCESS != (ret = instantiateCharacterArray(stringArg, utf8Length, FALSE, (CHAR_ARRAY_INSTANCE *) extractObjectFieldObjectPtr(pInstance->pCharArray)))) {
        return ret;
    }
    insertPrimitiveFieldInt(pInstance->offset, 0);
    insertPrimitiveFieldInt(pInstance->length, getArrayLength(extractObjectFieldObject(pInstance->pCharArray)));
    return SUCCESS;
}

RETURN_CODE instantiateInternedString(const char *stringArg, UINT16 utf8Length, OBJECT_INSTANCE *ppResult)
{
    RETURN_CODE ret;
    STRING_INSTANCE pInstance;
    
    UINT16 size = getInternedStringInstanceSize();

    if(SUCCESS != (ret = instantiateObjectGeneric(pJavaLangString, pJavaLangStringDef, getStringObjectFieldCount(), 
        ppResult, size, TRUE))) 
        {
        return ret;
    }
    pInstance = getStringInstanceFromObjectInstance(*ppResult);
    if(SUCCESS != (ret = instantiateCharacterArray(stringArg, utf8Length, TRUE, (CHAR_ARRAY_INSTANCE *) extractObjectFieldObjectPtr(pInstance->pCharArray)))) {
        return ret;
    }
    insertPrimitiveFieldInt(pInstance->offset, 0);
    insertPrimitiveFieldInt(pInstance->length, getArrayLength(extractObjectFieldObject(pInstance->pCharArray)));
    return SUCCESS;
}

