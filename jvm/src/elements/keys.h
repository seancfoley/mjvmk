#ifndef KEYS_H
#define KEYS_H


#include "types.h"


/*
 * access flags for classes and interfaces
 */
#define ACC_PUBLIC          0x0001
#define ACC_FINAL           0x0010
#define ACC_SUPER           0x0020
#define ACC_INTERFACE       0x0200
#define ACC_ABSTRACT        0x0400

/* proprietary class and interface flags */
#define ACC_INIT            0x4000  /* class has an initializer */

/*
 * access flags for fields
 * ACC_PUBLIC, ACC_FINAL, ACC_INIT: field has an initializer
 */
#define ACC_PRIVATE         0x0002
#define ACC_PROTECTED       0x0004
#define ACC_STATIC          0x0008
#define ACC_VOLATILE        0x0040
#define ACC_TRANSIENT       0x0080

/* proprietary field flags */
#define ACC_DOUBLE          0x8000  /* field is a long or a double */
#define ACC_PRIMITIVE       0x2000  /* field is primitive */

/*
 * access flags for methods
 * ACC_PUBLIC, ACC_FINAL, ACC_PROTECTED, ACC_PRIVATE, ACC_STATIC, ACC_ABSTRACT, ACC_INIT: method is init or clinit
 */
#define ACC_SYNCHRONIZED    0x0020
#define ACC_NATIVE_LINKED   0x0040
#define ACC_NATIVE          0x0100
#define ACC_FILTERED        0x0200
#define ACC_STRICT          0x0800



/* for convenience, we offer combinations */
#define ACC_FINAL_ABSTRACT                                  0x0410
#define ACC_FINAL_VOLATILE                                  0x0050
#define ACC_PUBLIC_STATIC_FINAL                             0x0019
#define ACC_PUBLIC_PROTECTED                                0x0005
#define ACC_PUBLIC_ABSTRACT                                 0x0401
#define ACC_STRICT_NATIVE_FINAL_SYNCHRONIZED_PRIVATE_STATIC 0x093A
#define ACC_ABSTRACT_NATIVE_FINAL_SYNCHRONIZED_STATIC       0x0538
#define ACC_INIT_PRIVATE                                    0x4002
#define ACC_PUBLIC_FINAL_SUPER                              0x0031
#define ACC_FINAL_SUPER                                     0x0030
#define ACC_PUBLIC_INIT                                     0x4001
#define ACC_DOUBLE_PRIMITIVE                                0xA000

/* only the low 11 bits of a 16-bit string key are used by the UTF8 string pool, so when storing a key
 * elsewhere the other bits may be used to store other information
 */
typedef UINT16 NameKey;
typedef UINT16 TypeKey;
typedef UINT32 NameTypeKey;
typedef UINT32 NamePackageKey;



/* the top 4 bits of the package key are used to record array depth, 
 * the next bit is an indicator of a primitive array class, and the lower 11
 * bits store the package's string key
 */
typedef UINT16 PackageKey;

#define ARRAY_DEPTH_SHIFT 12
#define PRIMITIVE_ARRAY_BIT 0x0800


#define PACKAGE_KEY_MASK 0x07ff
#define ARRAY_DEPTH_MASK 0xf000
#define PRIMITIVE_ARRAY_MASK 0x1800
#define PRIMITIVE_BIT_AND_DEPTH_MASK 0xf800
#define PACKAGE_KEY_PRIMITIVE_BIT_MASK 0x0fff
#define isPrimitiveArrayPackageKey(packageKey) ((packageKey) & PRIMITIVE_ARRAY_BIT)
#define addArrayDepthToPackageKey(packageKey, depth) ((PackageKey) ((packageKey) | ((depth) << ARRAY_DEPTH_SHIFT)))
#define getArrayDepth(packageKey) (((packageKey) & ARRAY_DEPTH_MASK) >> ARRAY_DEPTH_SHIFT)
#define getPackageStringKey(packageKey) ((PackageKey) (packageKey & PACKAGE_KEY_MASK))
#define getPackageStringAndPrimitiveBitKey(packageKey) ((PackageKey) (packageKey & PACKAGE_KEY_PRIMITIVE_BIT_MASK))

struct nameTypeStruct {
    TypeKey typeKey; 
    NameKey nameKey;
};

struct namePackageStruct {
    NameKey nameKey;
    NameKey packageKey;
};

typedef union NameType { /* used for methods and fields */
     NameTypeKey nameTypeKey; /* use this member to access both members at once */
     struct nameTypeStruct nt;
} NameType;

typedef union NamePackage { /* used for classes and interfaces */
     NamePackageKey namePackageKey; /* use this member to access both members at once */
     struct namePackageStruct np;
} NamePackage;



#endif
