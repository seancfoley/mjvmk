
#ifndef RESOURCE_H
#define RESOURCE_H

#include "elements/base.h"

#define MAX_PROPERTY_LINE_LEN 50

RETURN_CODE getProperty(char *key, UINT16 *propertyLength, char line[MAX_PROPERTY_LINE_LEN], char **pProperty);
RETURN_CODE getResourceAsStream(CLASS_INSTANCE pClassInstance, char *pResourceName, UINT16 resourceNameLength, OBJECT_INSTANCE *ppStreamObject);

#define getFileAccessObjectPtr(pResourceInputStreamObject)                                  \
    extractObjectFieldObjectPtr((pResourceInputStreamObject)->fileAccess)

#define getFileAccessObject(pResourceInputStreamObject)                                     \
    extractObjectFieldObject((pResourceInputStreamObject)->fileAccess)

#define getResourceFileAccess(pResourceInputStreamObject)                                   \
    (*(FILE_ACCESS *) &(getFileAccessObject(pResourceInputStreamObject)->primitiveFields))

#define insertResourceFileAccess(pResourceInputStreamObject, pFileAccess)                   \
    (getResourceFileAccess(pResourceInputStreamObject) = (pFileAccess))

#define instantiateFileAccessObject(pResourceInputStreamObject)                                                 \
    instantiateObject(pJavaLangObject, getFileAccessObjectPtr(pResourceInputStreamObject),  \
        (UINT16) (getInstantiateSize(pJavaLangObjectDef) + sizeof(FILE_ACCESS)), FALSE)


#endif

