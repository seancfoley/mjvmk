
#ifndef STRINGPOOL_H
#define STRINGPOOL_H

#include "elements/base.h"
#include "interpret/returnCodes.h"



typedef struct UTF8StringHashEntry { 
    struct UTF8StringHashEntry *pNext; /* next entry at the same index */
    NameKey nameKey;
    UINT16 length;
    char string[];          /* The characters of the string */
} UTF8StringHashEntry, *UTF8_STRING;

typedef struct UTF8TypeStringHashEntry { 
    struct UTF8StringHashEntry *pNext; /* next entry at the same index */
    TypeKey typeKey;
} UTF8TypeStringHashEntry, *UTF8_TYPE_STRING;


#define getUTF8StringHashEntrySize(n) (structOffsetOf(UTF8StringHashEntry, string) + n) /* note that there is no terminating null character */

RETURN_CODE addGlobalUTF8String(UTF8_STRING pEntry);

/* Either add the string to the pool or get the existing equivalent string in the pool */
RETURN_CODE getUTF8StringByChars(const char *pChar, UINT16 stringLength, UTF8_STRING *ppString);


/* Get a string out of the pool and its length */
char *getUTF8StringByKey(NameKey key, UINT16 *pLength);


/* special handling for package strings */
#define getPackageNameString(pChar, stringLength, ppString) getUTF8StringByChars(pChar, stringLength, (UTF8_STRING *) (ppString))
#define getPackageNameStringByKey(packageKey, pLength) getUTF8StringByKey(getPackageStringKey(packageKey), pLength)

/* special handling for type strings */
#define getMethodTypeString(pChar, stringLength, ppString) getUTF8StringByChars(pChar, stringLength, (UTF8_STRING *) (ppString))
#define getMethodTypeStringByKey(key, pLength) getUTF8StringByKey(key, pLength)

#define getFieldTypeString(pChar, stringLength, ppString) getUTF8StringByChars(pChar, stringLength, (UTF8_STRING *) (ppString))
#define getFieldTypeStringByKey(key, pLength) getUTF8StringByKey(key, pLength)



/* interned strings */

RETURN_CODE internString(const char *utf8string, UINT16 length, INTERNED_STRING_INSTANCE *ppString);
RETURN_CODE initializeStringPools();



/* special handling for class name strings */

typedef struct classIdentifierStruct {
    NamePackage key;
    const char *packageName;
    UINT16 packageNameLength;
    const char *name;
    UINT16 nameLength;
} classIdentifierStruct, *CLASS_ID;


RETURN_CODE getClassParams(const char *fullName, UINT16 fullNameLength, CLASS_ID pClassID);

void parseClassName(const char *fullName, UINT16 fullNameLength, const char **packageName, 
                    UINT16 *packageNameLength, const char **name, UINT16 *nameLength);


#endif
