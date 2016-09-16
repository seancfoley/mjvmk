#ifndef INSTANTIATE_H
#define INSTANTIATE_H

#include "elements/base.h"
#include "interpret/returnCodes.h"


/* single dimensional arrays */

RETURN_CODE instantiateArrayGeneric(CLASS_INSTANCE, ARRAY_CLASS_DEF, UINT16 elementByteSize, UINT32 length, 
                             BOOLEAN permanent, BOOLEAN isObjectArray, ARRAY_INSTANCE *ppInstance);

#define instantiateArray(pClassInstance, length, permanent, isObjectArray, ppInstance)          \
    (FIRST_ASSERT(isArrayClass((pClassInstance)->pRepresentativeClassDef))                      \
    instantiateArrayGeneric(pClassInstance,                                                     \
        (ARRAY_CLASS_DEF) (pClassInstance)->pRepresentativeClassDef,                            \
        ((ARRAY_CLASS_DEF) (pClassInstance)->pRepresentativeClassDef)->elementByteSize,         \
        length, permanent, isObjectArray, ppInstance))

#define instantiateCollectibleObjectArray(pClassInstance, length, ppInstance)                   \
    instantiateArray(pClassInstance, length, FALSE, TRUE, ppInstance)

#define instantiateCollectiblePrimitiveArray(pClassInstance, length, ppInstance)                \
    instantiateArray(pClassInstance, length, FALSE, FALSE, ppInstance)



/* multi-dimensional arrays */


RETURN_CODE instantiateMultiArrayGeneric(CLASS_INSTANCE pArrayClassInstance, 
                             ARRAY_CLASS_DEF pClassDef, UINT32 dimensions, 
                             JSTACK_FIELD dimensionLengths, BOOLEAN permanent, 
                             ARRAY_INSTANCE *ppInstance);


#define instantiateMultiArray(pClassInstance, dimensions, dimLengths, permanent, ppInstance)    \
    (FIRST_ASSERT(isArrayClass((pClassInstance)->pRepresentativeClassDef))                      \
    instantiateMultiArrayGeneric(pClassInstance,                                                \
        (ARRAY_CLASS_DEF) (pClassInstance)->pRepresentativeClassDef,                            \
        dimensions, dimLengths, permanent, ppInstance))


#define instantiateCollectibleMultiArray(pClassInstance, dimensions, dimLengths, ppInstance)    \
    instantiateMultiArray(pClassInstance, dimensions, dimLengths, FALSE, ppInstance)




/* non-array objects */

RETURN_CODE instantiateObjectGeneric(CLASS_INSTANCE, LOADED_CLASS_DEF, UINT16 objectFieldCount,
                      OBJECT_INSTANCE *ppInstance, UINT16 byteSize, BOOLEAN permanent);

#define instantiateObject(pClassInstance, ppInstance, byteSize, permanent)                      \
    (FIRST_ASSERT(isLoadedClass((pClassInstance)->pRepresentativeClassDef))                     \
    instantiateObjectGeneric(pClassInstance,                                                    \
            (LOADED_CLASS_DEF) ((pClassInstance)->pRepresentativeClassDef),                     \
            getObjectInstanceFieldCount((pClassInstance)->pRepresentativeClassDef),             \
            ppInstance, byteSize, permanent))


#define instantiateCollectibleObject(pClassInstance, ppInstance)                                \
    instantiateObject(pClassInstance, ppInstance,                                               \
    getInstantiateSize((pClassInstance)->pRepresentativeClassDef),                              \
    FALSE)

#define instantiatePermanentObject(pClassInstance, ppInstance)                                  \
    instantiateObject(pClassInstance, ppInstance,                                               \
    getInstantiateSize((pClassInstance)->pRepresentativeClassDef),                              \
    TRUE)




#endif