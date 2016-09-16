
#include "interpret/javaGlobals.h"
#include "object/instantiate.h"
#include "string/stringKeys.h"
#include "resolution/file/fileAccess.h"
#include "resolution/resolve/resolve.h"
#include "resource.h"


/* pProperty will point to the location in the line where the property begins */
RETURN_CODE getProperty(char *key, UINT16 *propertyLength, char line[MAX_PROPERTY_LINE_LEN], char **pProperty) 
{
    char propertiesFile[] = "System.properties";
    UINT16 propertiesFileLength = stringLength(propertiesFile);
    FILE_ACCESS pFileAccess;
    char *cp;
    UINT16 len;

    pFileAccess = openFile(propertiesFile, propertiesFileLength);
    if(pFileAccess == NULL) {
        *pProperty = NULL;
        return ERROR_CODE_VIRTUAL_MACHINE; /* the properties file is supposed to be there */
    }
    if(pFileAccess->errorFlag != SUCCESS) {
        *pProperty = NULL;
        return pFileAccess->errorFlag;
    }
    while(NULL != pFileAccess->pFileAccessMethods->loadLine(pFileAccess, MAX_PROPERTY_LINE_LEN, line)) {
        cp = stringChar(line, '=');
        if(cp == NULL) {
            continue;
        }
        if(memoryCompare(key, line, cp-line) == 0) {
            ++cp;
            len = stringLength(cp);
            while(cp[len - 1] == '\n' || cp[len - 1] == '\r') {
                len--;
                cp[len] = '\0';
            }
            *propertyLength = len;
            pFileAccess->pFileAccessMethods->close(pFileAccess);
            *pProperty = cp;

            return SUCCESS;
        }
    }
    *pProperty = NULL;
    return SUCCESS;
}

/* ppStreamObject will point to the newly created input stream object */
RETURN_CODE getResourceAsStream(CLASS_INSTANCE pBaseClassInstance, char *pResourceName, UINT16 resourceNameLength, OBJECT_INSTANCE *ppStreamObject)
{
    FILE_ACCESS pFileAccess;
    RESOURCE_INPUT_STREAM_INSTANCE pResourceInputStreamObject;
    jumpBufferStruct errorJump;
    RETURN_CODE ret;
    CLASS_INSTANCE pClassInstance;

    /* get the file access pointer */
    pFileAccess = openFile(pResourceName, resourceNameLength);
    if(pFileAccess == NULL) {
        *ppStreamObject = NULL;
        return SUCCESS;
    }
    pFileAccess->pErrorJump = &errorJump;
    if(setJump(errorJump) != 0) {
        return pFileAccess->errorFlag;
    }

    if(pFileAccess->errorFlag != SUCCESS) {
        *ppStreamObject = NULL;
        pFileAccess->pFileAccessMethods->close(pFileAccess);
        return pFileAccess->errorFlag;
    }

    /* load the ResourceInputStream class */
    ret = obtainClassRef(comMjvmkResourceInputStreamKey, &pClassInstance);
    if(ret != SUCCESS) {
        pFileAccess->pFileAccessMethods->close(pFileAccess);
        return ret;
    }

    /* instantiate the ResourceInputStream instance */
    ret = instantiateCollectibleObject(pClassInstance, ppStreamObject);
    if(ret != SUCCESS) {
        pFileAccess->pFileAccessMethods->close(pFileAccess);
        return ret;
    }
    pResourceInputStreamObject = getResourceInputStreamInstanceFromObjectInstance(*ppStreamObject);

    /* instantiate the file access object field */
    ret = instantiateFileAccessObject(pResourceInputStreamObject);
    if(ret != SUCCESS) {
        pFileAccess->pFileAccessMethods->close(pFileAccess);
        *ppStreamObject = NULL;
        return ret;
    }

    /* insert the file access pointer into the field */
    insertResourceFileAccess(pResourceInputStreamObject, pFileAccess);
    return SUCCESS;
}
