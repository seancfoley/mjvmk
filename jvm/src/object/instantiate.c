
#include "memory/memoryAlloc.h"
#include "interpret/javaGlobals.h"
#include "allocate.h"
#include "instantiate.h"

/* Routines for instantiation of objects: 
 *  - Array objects
 *  - Loaded class objects
 * 
 * Used to create:
 *   - System exception and error objects
 *   - System array objects
 *   - System string objects (as called by the instiation routines in javaString.c)
 *   - Objects to be returned by natives (eg Class.newInstance())
 *   - Objects created by the new, newarray, anewarray and multianewarray instructions
 *
 * 
 * java.lang.Class Objects are the only objects not created here, they are created by class preparation
 *
*/ 


RETURN_CODE instantiateArrayGeneric(CLASS_INSTANCE pArrayClassInstance, ARRAY_CLASS_DEF pClassDef, 
                                    UINT16 elementByteSize, UINT32 length, 
                                    BOOLEAN permanent, BOOLEAN isObjectArray, ARRAY_INSTANCE *ppInstance)
{
    ARRAY_INSTANCE pInstance;
    UINT32 byteSize;

    /* this VM stores array lengths in unsigned 16-bit integers, while the spec dictates that array size
     * is stored in signed 16 bit integers (negative array sizes result in exceptions), so we check here for overflow. 
     */
    if(length > (UINT16) -1) {
        return ERROR_CODE_VIRTUAL_MACHINE;
    }

    byteSize = getByteSizeOfArrayInstance(length, elementByteSize);
    if(permanent) {
        if((*ppInstance = memoryCalloc(byteSize, 1)) == NULL) {
            return ERROR_CODE_OUT_OF_MEMORY;
        }
    }
    else {
        if(collectibleMemoryCallocArray(byteSize, (UINT16) (isObjectArray ? length : 0), ppInstance) != SUCCESS) {
            return ERROR_CODE_OUT_OF_MEMORY;
        }
    }

    pInstance = *ppInstance;
    pInstance->header.header.pClassDef = (COMMON_CLASS_DEF) pClassDef;
    pInstance->header.header.pClassInstance = pArrayClassInstance;
    pInstance->header.arrayLength = length;  
    return SUCCESS;
}


RETURN_CODE instantiateMultiArrayGeneric(CLASS_INSTANCE pArrayClassInstance, ARRAY_CLASS_DEF pClassDef, UINT32 dimensions,
                             JSTACK_FIELD arrayLengths, BOOLEAN permanent, ARRAY_INSTANCE *ppInstance)
{
    UINT32 currentDimension = 1;
    UINT32 j;
    RETURN_CODE ret;
    OBJECT_ARRAY_INSTANCE pCurrentArray;
    OBJECT_ARRAY_INSTANCE pNextArray;
    ARRAY_INSTANCE pElementArray;
    ARRAY_INSTANCE pPreviousElementArray;
    COMMON_CLASS_DEF pElementClassDef;
    CLASS_INSTANCE pElementClassInstance;
    BOOLEAN fillingSecondLastDimension;
    jint currentArrayLength;
    jint elementArrayLength;
    
    currentArrayLength = getStackFieldInt(arrayLengths);
    

    /* for each dimension we will maintain a linked list of array objects that have been created of that dimension */
    if(isPrimitiveArrayClass(pClassDef)) {
        ret = instantiateCollectiblePrimitiveArray(pArrayClassInstance, currentArrayLength, ppInstance);
    }
    else {
        ret = instantiateCollectibleObjectArray(pArrayClassInstance, currentArrayLength, ppInstance);
    }

    if(ret != SUCCESS) {
        return ret;
    }

    if(dimensions == 1) {
        return SUCCESS;
    }

    pCurrentArray = (OBJECT_ARRAY_INSTANCE) *ppInstance;
    pElementClassDef = ((ARRAY_CLASS_DEF) (pArrayClassInstance->pRepresentativeClassDef))->elementClass.pElementClass;
    
    pElementClassInstance = getClassInstanceFromClassDef(pElementClassDef);
    

    while(TRUE) {

        /* there are no previously created element arrays at this dimension yet */
        pPreviousElementArray = NULL;
        elementArrayLength = getStackFieldInt(arrayLengths + currentDimension);
        fillingSecondLastDimension = (currentDimension == dimensions - 1) || (elementArrayLength == 0);
        
        do { /* We fill all arrays at this dimension */
            
            /* The next array at this dimension is pointed to by the first element of the current array. */
            pNextArray = (OBJECT_ARRAY_INSTANCE) getObjectFieldObject(pCurrentArray->values);

            /* fill in each element of the current array with arrays of the next dimension */
            for(j=0; j < currentArrayLength; j++) {
                if(fillingSecondLastDimension && isPrimitiveArrayClass(pElementClassDef)) {
                    ret = instantiateCollectiblePrimitiveArray(pElementClassInstance, elementArrayLength, (ARRAY_INSTANCE *) getObjectFieldObjectPtr(pCurrentArray->values + j));
                }
                else {
                    ret = instantiateCollectibleObjectArray(pElementClassInstance, elementArrayLength, (ARRAY_INSTANCE *) getObjectFieldObjectPtr(pCurrentArray->values + j));
                }
                if(ret != SUCCESS) {
                    return ret;
                }
                
                /* We are creating a singly linked list amongst all new arrays created in the next dimension 
                 * by having the first element in each array point to the previous array.
                 *
                 * This is not required if the next dimension is the last since the elements need not be populated.
                 */
                if(!fillingSecondLastDimension) {
                    pElementArray = (ARRAY_INSTANCE) getObjectFieldObject(pCurrentArray->values + j);
                    insertObjectFieldObject(pElementArray->firstValue, (OBJECT_INSTANCE) pPreviousElementArray);
                    pPreviousElementArray = pElementArray;
                }

            }

            pCurrentArray = pNextArray;
        } while(pCurrentArray != NULL);

        if(fillingSecondLastDimension) {
            break;
        }
        
        /* we've finished creating the element arrays at the current dimension, we now
           move into the next by starting with the last array created */
        pCurrentArray = (OBJECT_ARRAY_INSTANCE) pElementArray;
        currentArrayLength = elementArrayLength;
        pElementClassDef = ((ARRAY_CLASS_DEF) pElementClassInstance->pRepresentativeClassDef)->elementClass.pElementClass;
        pElementClassInstance = getClassInstanceFromClassDef(pElementClassDef);
        currentDimension++;
    }
    
    return SUCCESS;
}

/* Note that the very first objects created with this function will have their pClassDef and pClassInstance 
 * header members set to NULL.  This is because the Object class must be loaded first, but it will have 
 * interned string objects in its contant pool, so these interned strings will be created before the String class
 * has been loaded.  The same is true when loading the interned strings in the String class.
 *
 * These interned string objects are all fixed later on, see the initializeLoader function for details.
 */

RETURN_CODE instantiateObjectGeneric(CLASS_INSTANCE pClassInstance, LOADED_CLASS_DEF pClassDef, UINT16 objectFieldCount, 
                                     OBJECT_INSTANCE *ppInstance, UINT16 byteSize, BOOLEAN permanent)
{
    if(permanent) {
        void *res = memoryCalloc(byteSize, 1);
        if(res == NULL) {
            return ERROR_CODE_OUT_OF_MEMORY;
        }
        *ppInstance = (OBJECT_INSTANCE) (((JOBJECT_FIELD) res) + objectFieldCount);
    }
    else {
        if(collectibleMemoryCallocObject(byteSize, objectFieldCount, ppInstance) != SUCCESS) {
            return ERROR_CODE_OUT_OF_MEMORY;
        }
    }
    (*ppInstance)->header.pClassDef = (COMMON_CLASS_DEF) pClassDef; 
    (*ppInstance)->header.pClassInstance = pClassInstance;
    return SUCCESS;
}

