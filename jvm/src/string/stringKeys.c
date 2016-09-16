#include "stringKeys.h"



NameType initNameType;
NameType clinitNameType;
NameType runNameType;
NameType mainNameType;
NamePackage javaLangObjectKey;
NamePackage javaLangClassKey;
NamePackage javaLangStringKey;
NamePackage javaLangBooleanKey;
NamePackage javaLangByteKey;
NamePackage javaLangIntegerKey;
NamePackage javaLangLongKey;
NamePackage javaLangShortKey;
NamePackage javaLangCharacterKey;
NamePackage javaLangMathKey;
NamePackage javaLangStringBufferKey;
NamePackage javaLangThreadKey;
NamePackage javaLangThrowableKey;
NamePackage javaLangSystemKey;
NamePackage javaLangRuntimeKey;
NamePackage javaLangStringArrayKey;
NamePackage charArrayKey;
NamePackage integerArrayKey;
NamePackage shortArrayKey;
NamePackage javaUtilVectorKey;
NamePackage javaUtilHashtableKey;
NamePackage comMjvmkStandardOutputStreamKey;
NamePackage comMjvmkStandardErrorStreamKey;
NamePackage comMjvmkStackTraceKey;
NamePackage comMjvmkResourceInputStreamKey;

NamePackage javaLangOutOfMemoryErrorKey;
NamePackage javaLangStackOverflowErrorKey;
NamePackage javaLangExceptionInInitializerErrorKey;
NamePackage javaLangErrorKey;
NamePackage javaLangVirtualMachineErrorKey;
NamePackage javaLangLinkageErrorKey;
NamePackage javaLangNoClassDefFoundErrorKey;

NamePackage javaLangClassCastExceptionKey;
NamePackage javaLangArrayIndexOutOfBoundsExceptionKey;
NamePackage javaLangArithmeticExceptionKey;
NamePackage javaLangNegativeArraySizeExceptionKey;
NamePackage javaLangInterruptedExceptionKey;
NamePackage javaLangInstantiationExceptionKey;
NamePackage javaLangIllegalAccessExceptionKey;
NamePackage javaLangArrayStoreExceptionKey;
NamePackage javaLangIllegalArgumentExceptionKey;
NamePackage javaLangIllegalThreadStateExceptionKey;
NamePackage javaLangStringIndexOutOfBoundsExceptionKey;
NamePackage javaLangIllegalMonitorStateExceptionKey;
NamePackage javaLangNullPointerExceptionKey;
NamePackage javaIOExceptionKey;


#define initializedUTF8String(length, chars) {NULL, 0, length, chars}

/* package, class and type names */

UTF8StringHashEntry emptyString = initializedUTF8String(0, 0);
UTF8StringHashEntry maxArrayDepthString = initializedUTF8String(16, "[[[[[[[[[[[[[[[[");
static UTF8StringHashEntry javaLangPackageString = initializedUTF8String(9, "java/lang");
static UTF8StringHashEntry javaIoPackageString = initializedUTF8String(7, "java/io");
static UTF8StringHashEntry javaUtilPackageString = initializedUTF8String(9, "java/util");
static UTF8StringHashEntry comMjvmkPackageString = initializedUTF8String(9, "com/mjvmk");
static UTF8StringHashEntry ObjectString = initializedUTF8String(6, "Object");
static UTF8StringHashEntry StringBufferString = initializedUTF8String(12, "StringBuffer");
static UTF8StringHashEntry ThreadString = initializedUTF8String(6, "Thread");
static UTF8StringHashEntry ThrowableString = initializedUTF8String(9, "Throwable");
static UTF8StringHashEntry OutOfMemoryErrorString = initializedUTF8String(16, "OutOfMemoryError");
static UTF8StringHashEntry StackOverflowErrorString = initializedUTF8String(18, "StackOverflowError");
static UTF8StringHashEntry SystemString = initializedUTF8String(6, "System");
static UTF8StringHashEntry IllegalMonitorStateExceptionString = initializedUTF8String(28, "IllegalMonitorStateException");
static UTF8StringHashEntry NullPointerExceptionString = initializedUTF8String(20, "NullPointerException");
static UTF8StringHashEntry StackTraceString = initializedUTF8String(10, "StackTrace");
static UTF8StringHashEntry ExceptionInInitializerErrorString = initializedUTF8String(27, "ExceptionInInitializerError");
static UTF8StringHashEntry BooleanString = initializedUTF8String(7, "Boolean");    
static UTF8StringHashEntry ByteString = initializedUTF8String(4, "Byte");
static UTF8StringHashEntry IntegerString = initializedUTF8String(7, "Integer");
static UTF8StringHashEntry LongString = initializedUTF8String(4, "Long");
static UTF8StringHashEntry ShortString = initializedUTF8String(5, "Short");
static UTF8StringHashEntry CharacterString = initializedUTF8String(9, "Character");
static UTF8StringHashEntry MathString = initializedUTF8String(4, "Math");
static UTF8StringHashEntry VectorString = initializedUTF8String(6, "Vector");
static UTF8StringHashEntry HashtableString = initializedUTF8String(9, "Hashtable");
static UTF8StringHashEntry ResourceInputStreamString = initializedUTF8String(19, "ResourceInputStream");
static UTF8StringHashEntry StandardOutputStreamString = initializedUTF8String(20, "StandardOutputStream");
static UTF8StringHashEntry StandardErrorStreamString = initializedUTF8String(19, "StandardErrorStream");
static UTF8StringHashEntry VirtualMachineErrorString = initializedUTF8String(19, "VirtualMachineError");
static UTF8StringHashEntry LinkageErrorString = initializedUTF8String(12, "LinkageError");
static UTF8StringHashEntry NoClassDefFoundErrorString = initializedUTF8String(20, "NoClassDefFoundError");
static UTF8StringHashEntry ClassCastExceptionString = initializedUTF8String(18, "ClassCastException");
static UTF8StringHashEntry ArrayIndexOutOfBoundsExceptionString = initializedUTF8String(30, "ArrayIndexOutOfBoundsException");
static UTF8StringHashEntry ArithmeticExceptionString = initializedUTF8String(19, "ArithmeticException");
static UTF8StringHashEntry NegativeArraySizeExceptionString = initializedUTF8String(26, "NegativeArraySizeException");
static UTF8StringHashEntry InterruptedExceptionString = initializedUTF8String(20, "InterruptedException");
static UTF8StringHashEntry InstantiationExceptionString = initializedUTF8String(22, "InstantiationException");
static UTF8StringHashEntry IllegalAccessExceptionString = initializedUTF8String(22, "IllegalAccessException");
static UTF8StringHashEntry ArrayStoreExceptionString = initializedUTF8String(19, "ArrayStoreException");
static UTF8StringHashEntry IllegalArgumentExceptionString = initializedUTF8String(24, "IllegalArgumentException");
static UTF8StringHashEntry IllegalThreadStateExceptionString = initializedUTF8String(27, "IllegalThreadStateException");
static UTF8StringHashEntry StringIndexOutOfBoundsExceptionString = initializedUTF8String(31, "StringIndexOutOfBoundsException");
static UTF8StringHashEntry IOExceptionString = initializedUTF8String(11, "IOException");
static UTF8StringHashEntry ClassString = initializedUTF8String(5, "Class");
static UTF8StringHashEntry RuntimeString = initializedUTF8String(7, "Runtime");
static UTF8StringHashEntry StringString = initializedUTF8String(6, "String");
static UTF8StringHashEntry ErrorString = initializedUTF8String(5, "Error");
static UTF8StringHashEntry charString = initializedUTF8String(1, "C");
static UTF8StringHashEntry integerString = initializedUTF8String(1, "I");
static UTF8StringHashEntry shortString = initializedUTF8String(1, "S");


/* method names */


static UTF8StringHashEntry initString = initializedUTF8String(6, "<init>");
static UTF8StringHashEntry clinitString = initializedUTF8String(8, "<clinit>");
static UTF8StringHashEntry runString = initializedUTF8String(3, "run");
UTF8StringHashEntry mainString = initializedUTF8String(4, "main");
UTF8StringHashEntry getClassString = initializedUTF8String(8, "getClass");
UTF8StringHashEntry hashCodeString = initializedUTF8String(8, "hashCode");
UTF8StringHashEntry notifyString = initializedUTF8String(6, "notify");
UTF8StringHashEntry notifyAllString = initializedUTF8String(9, "notifyAll");
UTF8StringHashEntry waitString = initializedUTF8String(4, "wait");
UTF8StringHashEntry isInterfaceString = initializedUTF8String(11, "isInterface");
UTF8StringHashEntry isArrayString = initializedUTF8String(7, "isArray");
UTF8StringHashEntry isInstanceString = initializedUTF8String(10, "isInstance");
UTF8StringHashEntry isAssignableFromString = initializedUTF8String(16, "isAssignableFrom");
UTF8StringHashEntry getNameString = initializedUTF8String(7, "getName");
UTF8StringHashEntry newInstanceString = initializedUTF8String(11, "newInstance");
UTF8StringHashEntry forNameString = initializedUTF8String(7, "forName");
UTF8StringHashEntry getResourceAsStreamString = initializedUTF8String(19, "getResourceAsStream");
UTF8StringHashEntry printStackTraceString = initializedUTF8String(15, "printStackTrace");
UTF8StringHashEntry createStackTraceString = initializedUTF8String(16, "createStackTrace");
UTF8StringHashEntry identityHashCodeString = initializedUTF8String(16, "identityHashCode");
UTF8StringHashEntry currentTimeMillisString = initializedUTF8String(17, "currentTimeMillis");
UTF8StringHashEntry arraycopyString = initializedUTF8String(9, "arraycopy");
UTF8StringHashEntry getPropertyString = initializedUTF8String(11, "getProperty");
UTF8StringHashEntry freeMemoryString = initializedUTF8String(10, "freeMemory");
UTF8StringHashEntry totalMemoryString = initializedUTF8String(11, "totalMemory");
UTF8StringHashEntry gcString = initializedUTF8String(2, "gc");
UTF8StringHashEntry exitString = initializedUTF8String(4, "exit");
UTF8StringHashEntry lengthString = initializedUTF8String(6, "length");
UTF8StringHashEntry charAtString = initializedUTF8String(6, "charAt");
UTF8StringHashEntry charAtInternalString = initializedUTF8String(14, "charAtInternal");
UTF8StringHashEntry booleanValueString = initializedUTF8String(12, "booleanValue");
UTF8StringHashEntry byteValueString = initializedUTF8String(9, "byteValue");
UTF8StringHashEntry shortValueString = initializedUTF8String(10, "shortValue");
UTF8StringHashEntry charValueString = initializedUTF8String(9, "charValue");
UTF8StringHashEntry intValueString = initializedUTF8String(8, "intValue");
UTF8StringHashEntry minString = initializedUTF8String(3, "min");
UTF8StringHashEntry maxString = initializedUTF8String(3, "max");
UTF8StringHashEntry absString = initializedUTF8String(3, "abs");
UTF8StringHashEntry startString = initializedUTF8String(5, "start");
UTF8StringHashEntry yieldString = initializedUTF8String(5, "yield");
UTF8StringHashEntry currentThreadString = initializedUTF8String(13, "currentThread");
UTF8StringHashEntry sleepString = initializedUTF8String(5, "sleep");
UTF8StringHashEntry isAliveString = initializedUTF8String(7, "isAlive");
UTF8StringHashEntry setPriorityString = initializedUTF8String(11, "setPriority");
UTF8StringHashEntry getPriorityString = initializedUTF8String(11, "getPriority");
UTF8StringHashEntry activeCountString = initializedUTF8String(11, "activeCount");
UTF8StringHashEntry joinString = initializedUTF8String(4, "join");
UTF8StringHashEntry interruptString = initializedUTF8String(9, "interrupt");
UTF8StringHashEntry sizeString = initializedUTF8String(4, "size");
UTF8StringHashEntry internalElementAtString = initializedUTF8String(17, "internalElementAt");
UTF8StringHashEntry writeString = initializedUTF8String(5, "write");
UTF8StringHashEntry flushString = initializedUTF8String(5, "flush");
UTF8StringHashEntry readString = initializedUTF8String(4, "read");
UTF8StringHashEntry closeString = initializedUTF8String(5, "close");



/* method types */

UTF8StringHashEntry aJavaLangStringVType = initializedUTF8String(22, "([Ljava/lang/String;)V");
UTF8StringHashEntry vType = initializedUTF8String(3, "()V");
UTF8StringHashEntry javaLangClassType = initializedUTF8String(19, "()Ljava/lang/Class;");
UTF8StringHashEntry iType = initializedUTF8String(3, "()I");
UTF8StringHashEntry sType = initializedUTF8String(3, "()S");
UTF8StringHashEntry cType = initializedUTF8String(3, "()C");
UTF8StringHashEntry bType = initializedUTF8String(3, "()B");
UTF8StringHashEntry zType = initializedUTF8String(3, "()Z");
UTF8StringHashEntry jType = initializedUTF8String(3, "()J");
UTF8StringHashEntry iiType = initializedUTF8String(4, "(I)I");
UTF8StringHashEntry jjType = initializedUTF8String(4, "(J)J");
UTF8StringHashEntry ivType = initializedUTF8String(4, "(I)V");
UTF8StringHashEntry jvType = initializedUTF8String(4, "(J)V");
UTF8StringHashEntry jivType = initializedUTF8String(5, "(JI)V");
UTF8StringHashEntry icType = initializedUTF8String(4, "(I)C");
UTF8StringHashEntry iiiType = initializedUTF8String(5, "(II)I");
UTF8StringHashEntry jjjType = initializedUTF8String(5, "(JJ)J");
UTF8StringHashEntry abiivType = initializedUTF8String(7, "([BII)V");
UTF8StringHashEntry abvType = initializedUTF8String(5, "([B)V");
UTF8StringHashEntry iJavaLangObjectType = initializedUTF8String(21, "(I)Ljava/lang/Object;");
UTF8StringHashEntry javaLangObjectZType = initializedUTF8String(21, "(Ljava/lang/Object;)Z");
UTF8StringHashEntry javaLangClassZType = initializedUTF8String(20, "(Ljava/lang/Class;)Z");
UTF8StringHashEntry javaLangStringType = initializedUTF8String(20, "()Ljava/lang/String;");
UTF8StringHashEntry javaLangThreadType = initializedUTF8String(20, "()Ljava/lang/Thread;");
UTF8StringHashEntry javaLangObjectType = initializedUTF8String(20, "()Ljava/lang/Object;");
UTF8StringHashEntry javaLangStringJavaLangClassType = initializedUTF8String(37, "(Ljava/lang/String;)Ljava/lang/Class;");
UTF8StringHashEntry javaLangStringjavaIoInputStreamType = initializedUTF8String(41, "(Ljava/lang/String;)Ljava/io/InputStream;");
UTF8StringHashEntry javaLangObjectIType = initializedUTF8String(21, "(Ljava/lang/Object;)I");
UTF8StringHashEntry javaLangObjectIjavaLangObjectIIVType = initializedUTF8String(42, "(Ljava/lang/Object;ILjava/lang/Object;II)V");
UTF8StringHashEntry javaLangStringjavaLangStringType = initializedUTF8String(38, "(Ljava/lang/String;)Ljava/lang/String;");



/* This function should be called for all new string pools */
void initializeGlobalStrings()
{
    addGlobalUTF8String(&initString);
    initNameType.nt.nameKey = initString.nameKey;

    addGlobalUTF8String(&clinitString);
    clinitNameType.nt.nameKey = clinitString.nameKey;

    addGlobalUTF8String(&runString);
    runNameType.nt.nameKey = runString.nameKey;

    addGlobalUTF8String(&mainString);
    mainNameType.nt.nameKey = mainString.nameKey;

    addGlobalUTF8String(&aJavaLangStringVType);
    mainNameType.nt.typeKey = aJavaLangStringVType.nameKey;

    addGlobalUTF8String(&vType);
    runNameType.nt.typeKey = 
        initNameType.nt.typeKey = 
        clinitNameType.nt.typeKey = 
        vType.nameKey;

    addGlobalUTF8String(&javaLangPackageString);
    javaLangStringKey.np.packageKey = 
        javaLangBooleanKey.np.packageKey =
        javaLangByteKey.np.packageKey =
        javaLangIntegerKey.np.packageKey =
        javaLangLongKey.np.packageKey =
        javaLangShortKey.np.packageKey =
        javaLangCharacterKey.np.packageKey =
        javaLangMathKey.np.packageKey =
        javaLangStringBufferKey.np.packageKey =
        javaLangObjectKey.np.packageKey = 
        javaLangClassKey.np.packageKey = 
        javaLangThreadKey.np.packageKey = 
        javaLangThrowableKey.np.packageKey = 
        javaLangOutOfMemoryErrorKey.np.packageKey = 
        javaLangStackOverflowErrorKey.np.packageKey = 
        javaLangSystemKey.np.packageKey = 
        javaLangRuntimeKey.np.packageKey = 
        javaLangIllegalMonitorStateExceptionKey.np.packageKey =
        javaLangNullPointerExceptionKey.np.packageKey =
        javaLangErrorKey.np.packageKey = 
        javaLangExceptionInInitializerErrorKey.np.packageKey = 
        javaLangVirtualMachineErrorKey.np.packageKey = 
        javaLangLinkageErrorKey.np.packageKey = 
        javaLangNoClassDefFoundErrorKey.np.packageKey = 
        javaLangClassCastExceptionKey.np.packageKey = 
        javaLangArrayIndexOutOfBoundsExceptionKey.np.packageKey = 
        javaLangArithmeticExceptionKey.np.packageKey = 
        javaLangNegativeArraySizeExceptionKey.np.packageKey = 
        javaLangInterruptedExceptionKey.np.packageKey = 
        javaLangInstantiationExceptionKey.np.packageKey = 
        javaLangIllegalAccessExceptionKey.np.packageKey = 
        javaLangArrayStoreExceptionKey.np.packageKey = 
        javaLangIllegalArgumentExceptionKey.np.packageKey = 
        javaLangIllegalThreadStateExceptionKey.np.packageKey = 
        javaLangStringIndexOutOfBoundsExceptionKey.np.packageKey = 
        javaLangStringArrayKey.np.packageKey = 
        javaLangPackageString.nameKey;

    addGlobalUTF8String(&javaUtilPackageString);
    javaUtilVectorKey.np.packageKey = 
        javaUtilHashtableKey.np.packageKey =
        javaUtilPackageString.nameKey;

    addGlobalUTF8String(&javaIoPackageString);
    javaIOExceptionKey.np.packageKey = 
        javaIoPackageString.nameKey;

    addGlobalUTF8String(&comMjvmkPackageString);
        comMjvmkStackTraceKey.np.packageKey = 
        comMjvmkStandardOutputStreamKey.np.packageKey = 
        comMjvmkStandardErrorStreamKey.np.packageKey = 
        comMjvmkResourceInputStreamKey.np.packageKey = 
        comMjvmkPackageString.nameKey;

    addGlobalUTF8String(&maxArrayDepthString);
    addGlobalUTF8String(&emptyString);
    charArrayKey.np.packageKey = 
    integerArrayKey.np.packageKey =
    shortArrayKey.np.packageKey =
        emptyString.nameKey | PRIMITIVE_ARRAY_BIT;

    charArrayKey.np.packageKey = addArrayDepthToPackageKey(charArrayKey.np.packageKey, 1);    
    integerArrayKey.np.packageKey = addArrayDepthToPackageKey(integerArrayKey.np.packageKey, 1);    
    shortArrayKey.np.packageKey = addArrayDepthToPackageKey(shortArrayKey.np.packageKey, 1);    
    javaLangStringArrayKey.np.packageKey = addArrayDepthToPackageKey(javaLangStringArrayKey.np.packageKey, 1);    
    
    addGlobalUTF8String(&ObjectString);
    javaLangObjectKey.np.nameKey = ObjectString.nameKey;
    
    addGlobalUTF8String(&ClassString);
    javaLangClassKey.np.nameKey = ClassString.nameKey;
    
    addGlobalUTF8String(&StringString);
    javaLangStringKey.np.nameKey = StringString.nameKey;

    addGlobalUTF8String(&BooleanString);
    javaLangBooleanKey.np.nameKey = BooleanString.nameKey;
    
    addGlobalUTF8String(&ByteString);
    javaLangByteKey.np.nameKey = ByteString.nameKey;
    
    addGlobalUTF8String(&IntegerString);
    javaLangIntegerKey.np.nameKey = IntegerString.nameKey;
    
    addGlobalUTF8String(&LongString);
    javaLangLongKey.np.nameKey = LongString.nameKey;
    
    addGlobalUTF8String(&ShortString);
    javaLangShortKey.np.nameKey = ShortString.nameKey;
    
    addGlobalUTF8String(&CharacterString);
    javaLangCharacterKey.np.nameKey = CharacterString.nameKey;   

    addGlobalUTF8String(&MathString);
    javaLangMathKey.np.nameKey = MathString.nameKey;   

    addGlobalUTF8String(&StringBufferString);
    javaLangStringBufferKey.np.nameKey = StringBufferString.nameKey;
    
    addGlobalUTF8String(&ThrowableString);
    javaLangThrowableKey.np.nameKey = ThrowableString.nameKey;
    
    addGlobalUTF8String(&ThreadString);
    javaLangThreadKey.np.nameKey = ThreadString.nameKey;

    addGlobalUTF8String(&StackTraceString);
    comMjvmkStackTraceKey.np.nameKey = StackTraceString.nameKey;
    
    addGlobalUTF8String(&OutOfMemoryErrorString);
    javaLangOutOfMemoryErrorKey.np.nameKey = OutOfMemoryErrorString.nameKey;
    
    addGlobalUTF8String(&StackOverflowErrorString);
    javaLangStackOverflowErrorKey.np.nameKey = StackOverflowErrorString.nameKey;
    
    addGlobalUTF8String(&SystemString);
    javaLangSystemKey.np.nameKey = SystemString.nameKey;
    
    addGlobalUTF8String(&RuntimeString);
    javaLangRuntimeKey.np.nameKey = RuntimeString.nameKey;
    
    addGlobalUTF8String(&IllegalMonitorStateExceptionString);
    javaLangIllegalMonitorStateExceptionKey.np.nameKey = IllegalMonitorStateExceptionString.nameKey;
    
    addGlobalUTF8String(&NullPointerExceptionString);
    javaLangNullPointerExceptionKey.np.nameKey = NullPointerExceptionString.nameKey;
    
    addGlobalUTF8String(&ErrorString);
    javaLangErrorKey.np.nameKey = ErrorString.nameKey;

    addGlobalUTF8String(&ExceptionInInitializerErrorString);
    javaLangExceptionInInitializerErrorKey.np.nameKey = ExceptionInInitializerErrorString.nameKey;

    addGlobalUTF8String(&VirtualMachineErrorString);
    javaLangVirtualMachineErrorKey.np.nameKey = VirtualMachineErrorString.nameKey;

    addGlobalUTF8String(&LinkageErrorString);
    javaLangLinkageErrorKey.np.nameKey = LinkageErrorString.nameKey;

    addGlobalUTF8String(&NoClassDefFoundErrorString);
    javaLangNoClassDefFoundErrorKey.np.nameKey = NoClassDefFoundErrorString.nameKey;

    addGlobalUTF8String(&ClassCastExceptionString);
    javaLangClassCastExceptionKey.np.nameKey = ClassCastExceptionString.nameKey;

    addGlobalUTF8String(&ArrayIndexOutOfBoundsExceptionString);
    javaLangArrayIndexOutOfBoundsExceptionKey.np.nameKey = ArrayIndexOutOfBoundsExceptionString.nameKey;

    addGlobalUTF8String(&ArithmeticExceptionString);
    javaLangArithmeticExceptionKey.np.nameKey = ArithmeticExceptionString.nameKey;

    addGlobalUTF8String(&NegativeArraySizeExceptionString);
    javaLangNegativeArraySizeExceptionKey.np.nameKey = NegativeArraySizeExceptionString.nameKey;
    
    addGlobalUTF8String(&InterruptedExceptionString);
    javaLangInterruptedExceptionKey.np.nameKey = InterruptedExceptionString.nameKey;
    
    addGlobalUTF8String(&InstantiationExceptionString);
    javaLangInstantiationExceptionKey.np.nameKey = InstantiationExceptionString.nameKey;

    addGlobalUTF8String(&IllegalAccessExceptionString);
    javaLangIllegalAccessExceptionKey.np.nameKey = IllegalAccessExceptionString.nameKey;

    addGlobalUTF8String(&ArrayStoreExceptionString);
    javaLangArrayStoreExceptionKey.np.nameKey = ArrayStoreExceptionString.nameKey;

    addGlobalUTF8String(&IllegalArgumentExceptionString);
    javaLangIllegalArgumentExceptionKey.np.nameKey = IllegalArgumentExceptionString.nameKey;

    addGlobalUTF8String(&IllegalThreadStateExceptionString);
    javaLangIllegalThreadStateExceptionKey.np.nameKey = IllegalThreadStateExceptionString.nameKey;

    addGlobalUTF8String(&StringIndexOutOfBoundsExceptionString);
    javaLangStringIndexOutOfBoundsExceptionKey.np.nameKey = StringIndexOutOfBoundsExceptionString.nameKey;

    addGlobalUTF8String(&IOExceptionString);
    javaIOExceptionKey.np.nameKey = IOExceptionString.nameKey;

    javaLangStringArrayKey.np.nameKey = StringString.nameKey;
     
    addGlobalUTF8String(&charString);
    charArrayKey.np.nameKey = charString.nameKey;
    
    addGlobalUTF8String(&integerString);
    integerArrayKey.np.nameKey = integerString.nameKey;
    
    addGlobalUTF8String(&shortString);
    shortArrayKey.np.nameKey = shortString.nameKey;


    
    addGlobalUTF8String(&VectorString);
    javaUtilVectorKey.np.nameKey = VectorString.nameKey;
    
    addGlobalUTF8String(&HashtableString);
    javaUtilHashtableKey.np.nameKey = HashtableString.nameKey;
    
    addGlobalUTF8String(&StandardOutputStreamString);
    comMjvmkStandardOutputStreamKey.np.nameKey = StandardOutputStreamString.nameKey;
    
    addGlobalUTF8String(&ResourceInputStreamString);
    comMjvmkResourceInputStreamKey.np.nameKey = ResourceInputStreamString.nameKey;

    addGlobalUTF8String(&StandardErrorStreamString);
    comMjvmkStandardErrorStreamKey.np.nameKey = StandardErrorStreamString.nameKey;





    addGlobalUTF8String(&getClassString);
    addGlobalUTF8String(&hashCodeString);
    addGlobalUTF8String(&notifyString);
    addGlobalUTF8String(&notifyAllString);
    addGlobalUTF8String(&waitString);
    addGlobalUTF8String(&isInterfaceString);
    addGlobalUTF8String(&isArrayString);
    addGlobalUTF8String(&isInstanceString);
    addGlobalUTF8String(&isAssignableFromString);
    addGlobalUTF8String(&getNameString);
    addGlobalUTF8String(&newInstanceString);
    addGlobalUTF8String(&forNameString);
    addGlobalUTF8String(&getResourceAsStreamString);
    addGlobalUTF8String(&printStackTraceString);
    addGlobalUTF8String(&createStackTraceString);
    addGlobalUTF8String(&identityHashCodeString);
    addGlobalUTF8String(&currentTimeMillisString);
    addGlobalUTF8String(&arraycopyString);
    addGlobalUTF8String(&getPropertyString);
    addGlobalUTF8String(&freeMemoryString);
    addGlobalUTF8String(&totalMemoryString);
    addGlobalUTF8String(&gcString);
    addGlobalUTF8String(&exitString);
    addGlobalUTF8String(&lengthString);
    addGlobalUTF8String(&charAtString);
    addGlobalUTF8String(&charAtInternalString);
    addGlobalUTF8String(&booleanValueString);
    addGlobalUTF8String(&byteValueString);
    addGlobalUTF8String(&shortValueString);
    addGlobalUTF8String(&charValueString);
    addGlobalUTF8String(&intValueString);
    addGlobalUTF8String(&minString);
    addGlobalUTF8String(&maxString);
    addGlobalUTF8String(&absString);
    addGlobalUTF8String(&startString);
    addGlobalUTF8String(&yieldString);
    addGlobalUTF8String(&currentThreadString);
    addGlobalUTF8String(&sleepString);
    addGlobalUTF8String(&isAliveString);
    addGlobalUTF8String(&setPriorityString);
    addGlobalUTF8String(&getPriorityString);
    addGlobalUTF8String(&activeCountString);
    addGlobalUTF8String(&joinString);
    addGlobalUTF8String(&interruptString);
    addGlobalUTF8String(&sizeString);
    addGlobalUTF8String(&internalElementAtString);
    addGlobalUTF8String(&writeString);
    addGlobalUTF8String(&flushString);
    addGlobalUTF8String(&readString);
    addGlobalUTF8String(&closeString);
    
    addGlobalUTF8String(&javaLangClassType);
    addGlobalUTF8String(&iType);
    addGlobalUTF8String(&sType);
    addGlobalUTF8String(&cType);
    addGlobalUTF8String(&bType);
    addGlobalUTF8String(&zType);
    addGlobalUTF8String(&jType);
    addGlobalUTF8String(&iiType);
    addGlobalUTF8String(&jjType);
    addGlobalUTF8String(&ivType);
    addGlobalUTF8String(&jvType);
    addGlobalUTF8String(&jivType);
    addGlobalUTF8String(&icType);
    addGlobalUTF8String(&iiiType);
    addGlobalUTF8String(&jjjType);
    addGlobalUTF8String(&abiivType);
    addGlobalUTF8String(&abvType);
    addGlobalUTF8String(&iJavaLangObjectType);
    addGlobalUTF8String(&javaLangObjectZType);
    addGlobalUTF8String(&javaLangClassZType);
    addGlobalUTF8String(&javaLangStringType);
    addGlobalUTF8String(&javaLangThreadType);
    addGlobalUTF8String(&javaLangObjectType);
    addGlobalUTF8String(&javaLangStringJavaLangClassType);
    addGlobalUTF8String(&javaLangStringjavaIoInputStreamType);
    addGlobalUTF8String(&javaLangObjectIType);
    addGlobalUTF8String(&javaLangObjectIjavaLangObjectIIVType);
    addGlobalUTF8String(&javaLangStringjavaLangStringType);

    return;
}
