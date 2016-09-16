
#include "memory/memoryAlloc.h" 
#include "interpret/javaGlobals.h"
#include "javaString.h"
#include "stringKeys.h"
#include "stringPool.h"


/* Both the intern string pool and UTF8 string pool are implemented as hash tables */

typedef struct hashTableStruct {
    UINT16 entryCount;     /* Number of entries */
    UINT16 count;           /* Number of total items in the table */
    void *entries[1];        /* Array of entries, each one will be a UTF8_STRING or an INTERNED_STRING_INSTANCE */
} hashTableStruct;

#define getHashTableSize(numEntries) (structOffsetOf(hashTableStruct, entries) + (numEntries) * sizeof(void *))



HASH_TABLE createStringPool(UINT16 entryCount) 
{
    HASH_TABLE pTable;

    pTable = memoryCalloc(getHashTableSize(entryCount), 1);
    
    if(pTable == NULL) {
        return NULL;
    }
    pTable->entryCount = entryCount;
    return pTable;
}



#define getStringHash(valueVar, string, length) {                       \
    const char *end = string + length;                                  \
    const char *str = string;                                           \
    for(valueVar = 0; str < end; valueVar = valueVar * 37 + *str++);    \
}


 
RETURN_CODE getUTF8StringByChars(const char *string, UINT16 stringLength, UTF8_STRING *ppString)
{
    HASH_TABLE table = getUTF8StringPool();
    UINT16 index;
    UTF8_STRING *ppEntry;
    UTF8_STRING pEntry;
    char *utf8String;
    UINT16 length;

    getStringHash(index, string, stringLength);
    index %= table->entryCount;
    ppEntry = (UTF8_STRING *) (table->entries + index);
    pEntry = *ppEntry;
    if(pEntry != NULL) {
        
        /* Search the linked list for the corresponding string. */
        do {
            utf8String = pEntry->string;
            length = pEntry->length;
            if (length == stringLength && (utf8String == string || memoryCompare(utf8String, string, length) == 0)) {
                *ppString = pEntry;
                return SUCCESS;
            }
            pEntry = pEntry->pNext;
        } while(pEntry != NULL);
    }

    
    if((pEntry = memoryAlloc(getUTF8StringHashEntrySize(stringLength))) == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }

    memoryCopy(pEntry->string, string, stringLength); /* note that there is no terminating null character */
    pEntry->length = stringLength;
    
    enterCritical();

    /* Add to the hash table */

    pEntry->pNext = *ppEntry;
    pEntry->nameKey = table->entryCount + ((pEntry->pNext == NULL) ? index : pEntry->pNext->nameKey);
    *ppEntry = pEntry;
    *ppString = pEntry;
    table->count++;
    
    exitCritical();

    return SUCCESS;
}


char *getUTF8StringByKey(NameKey key, UINT16 *pLength) 
{ 
    HASH_TABLE table = getUTF8StringPool();
    UTF8_STRING pEntry;

    /* Search the linked list for the corresponding string. */
    for (pEntry = (UTF8_STRING) table->entries[key % table->entryCount]; pEntry != NULL; pEntry = pEntry->pNext) { 
        if (key == pEntry->nameKey) { 
            *pLength = pEntry->length;
            return pEntry->string;
        }
    }
    return NULL;
}

static BOOLEAN findMatchingUnicodeString(INTERNED_STRING_INSTANCE pJavaString, const char *utf8String, UINT16 length, INTERNED_STRING_INSTANCE* ppString)
{
    jchar *pUnicodeChar;
    BOOLEAN isMatch;
    UINT16 jstringLength;
    const char *pChar;
    UINT8 utf8CharLength;
    
    do {
        pChar = utf8String;
        jstringLength = 0;
        isMatch = TRUE;
        pUnicodeChar = ((CHAR_ARRAY_INSTANCE) extractObjectFieldObject(pJavaString->pCharArray))->values + extractPrimitiveFieldInt(pJavaString->offset);

        /* compare the characters, computing the utf8 string length as we go along */
        while (pChar < utf8String + length) { 
            if (getUnicodeCharFromUTF8Char(pChar, &utf8CharLength) != *pUnicodeChar) { 
                isMatch = FALSE;
                break;
            }
            jstringLength++;
            pUnicodeChar++;
            pChar += utf8CharLength;
        }

        if (isMatch) {

            if(jstringLength == extractPrimitiveFieldInt(pJavaString->length)) {
                *ppString = pJavaString;
                return TRUE;
            }
            else {

                /* continue testing but now we have the string length computed for quicker testing */
                /* we will arrive here if the first n characters of the unicode string matched all n characters of the utf8 string */
                /* this might occur more often than it seems, because the hashing already suggests that the characters might be similar */
                
                pJavaString = pJavaString->next;
                while(pJavaString != NULL) {
                    
                    if(jstringLength == extractPrimitiveFieldInt(pJavaString->length)) {
                        pChar = utf8String;
                        isMatch = TRUE;
                        pUnicodeChar = ((CHAR_ARRAY_INSTANCE) extractObjectFieldObject(pJavaString->pCharArray))->values + extractPrimitiveFieldInt(pJavaString->offset);

                        while (pChar < utf8String + length) { 
                            if (getUnicodeCharFromUTF8Char(pChar, &utf8CharLength) != *pUnicodeChar) {
                                isMatch = FALSE;
                                break;
                            }
                            pChar += utf8CharLength;
                            pUnicodeChar++;
                        }

                        if(isMatch) {
                            *ppString = pJavaString;
                            return TRUE;
                        }
                    }
                    pJavaString = pJavaString->next;
                }
                break;
            }
        }
        pJavaString = pJavaString->next; 
    } while(pJavaString != NULL);
    return FALSE;
}

RETURN_CODE internString(const char *utf8String, UINT16 length, INTERNED_STRING_INSTANCE *ppString)
{ 
    RETURN_CODE ret;
    OBJECT_INSTANCE pObject;
    UINT16 hashValue;
    HASH_TABLE table = getUnicodeStringPool();
    INTERNED_STRING_INSTANCE *ppEntry;
    INTERNED_STRING_INSTANCE pEntry;
    
    getStringHash(hashValue, utf8String, length);
    hashValue %= table->entryCount;
    ppEntry = (INTERNED_STRING_INSTANCE *) (table->entries + hashValue);
    pEntry = *ppEntry;
    if(pEntry != NULL && findMatchingUnicodeString(pEntry, utf8String, length, ppString)) {
        return SUCCESS;
    }

    ret = instantiateInternedString(utf8String, length, &pObject);
    if(ret != SUCCESS) {
        return ret;
    }
    pEntry = getInternedStringInstanceFromObjectInstance(pObject);

    enterCritical();

    pEntry->next = *ppEntry;
    *ppEntry = pEntry;
    *ppString = pEntry;
    table->count++;

    exitCritical();

    return SUCCESS;
}

void parseClassName(const char *fullName, UINT16 fullNameLength, const char **packageName, UINT16 *packageNameLength,
                           const char **name, UINT16 *nameLength)
{
    const char *cp;
    UINT16 packageLength;
    
    for(cp = fullName + (fullNameLength - 1); *cp != '/' && cp >= fullName; cp--);
    packageLength = ++cp - fullName;

    *name = fullName + packageLength;
    *nameLength = fullNameLength - packageLength;

    if(packageLength > 1) {
        *packageNameLength = packageLength - 1;
        *packageName = fullName;
    }
    else {
        *packageNameLength = 0;
        *packageName = NULL;
    }
    return;
}

RETURN_CODE getClassParams(const char *fullName, UINT16 fullNameLength, CLASS_ID pClassID)
{
    RETURN_CODE ret;
    UTF8_STRING pUtf8PoolString;
    UINT8 arrayDepth = 0;
    BOOLEAN isPrimitiveElementClass;

    if(*fullName == ARRAY_ARRAY_TYPE) {
        do {
            arrayDepth++;
            fullName++;
            fullNameLength--;
        } while(*fullName == ARRAY_ARRAY_TYPE);
    
        if(*fullName != OBJECT_ARRAY_TYPE) {
            /* we have a primitive element class */
            isPrimitiveElementClass = TRUE;
        }
        else {
            /* skip the starting 'L' and the final ';' */
            isPrimitiveElementClass = FALSE;
            fullName++;
            fullNameLength -= 2;
        }
    }

    parseClassName(fullName, fullNameLength, &pClassID->packageName, &pClassID->packageNameLength, 
        &pClassID->name, &pClassID->nameLength);
    
    if((ret = getUTF8StringByChars(pClassID->name, pClassID->nameLength, &pUtf8PoolString)) != SUCCESS) {
        return ret;
    }
    pClassID->key.np.nameKey = pUtf8PoolString->nameKey;
    if(pClassID->packageNameLength > 0) {
        ret = getPackageNameString(fullName, pClassID->packageNameLength, &pUtf8PoolString);
        pClassID->key.np.packageKey = pUtf8PoolString->nameKey;
    }
    else {
        pClassID->key.np.packageKey = emptyString.nameKey;
    }
    if(arrayDepth) {
        pClassID->key.np.packageKey = addArrayDepthToPackageKey(pClassID->key.np.packageKey, arrayDepth);
        if(isPrimitiveElementClass) {
            pClassID->key.np.packageKey |= PRIMITIVE_ARRAY_BIT;
        }
    }
    return ret;
}



/* keys that will be required by the system are generated here */

RETURN_CODE addGlobalUTF8String(UTF8_STRING pEntry)
{
    HASH_TABLE table = getUTF8StringPool();
    UINT16 index;
    UTF8_STRING *ppEntry;
    
    getStringHash(index, pEntry->string, pEntry->length);
    index %= table->entryCount;
    ppEntry = (UTF8_STRING *) (table->entries + index);

    enterCritical();

    /* we don't bother checking for duplicates, it is not necessary, and there should not be any as well */
    pEntry->pNext = *ppEntry;
    pEntry->nameKey = table->entryCount + ((pEntry->pNext == NULL) ? index : pEntry->pNext->nameKey);
    *ppEntry = pEntry;
    table->count++;
    
    exitCritical();

    return SUCCESS;
}



RETURN_CODE initializeStringPools() 
{
    HASH_TABLE pTable;

    pTable = createStringPool(UTF8_STRING_TABLE_SIZE);
    if(pTable == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    setUTF8StringPool(pTable);
    pTable = createStringPool(INTERN_STRING_TABLE_SIZE);
    if(pTable == NULL) {
        return ERROR_CODE_OUT_OF_MEMORY;
    }
    setUnicodeStringPool(pTable);
    return SUCCESS;
}







