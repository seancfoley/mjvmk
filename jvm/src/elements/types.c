
#include <math.h>
#include "types.h"

#if NATIVE_LONG

jlong jlongZero = {0, 0};
jlong jlongOne = {0, 1};

#define LOW_TWO_BYTES_MASK 0x0000ffff
#define HIGH_TWO_BYTES_MASK 0xffff0000

jlong jlongMultiply(jlong value1, jlong value2)
{
    jlong result;
    jlong tempSecond;
    jlong tempThird;
    jlong tempFourth;

    /* we break the values up into pieces to avoid overflow problems, then multiply all the pieces,
     * and then join up all the results
     */
    UINT32 first = (LOW_TWO_BYTES_MASK & value1.low) * (LOW_TWO_BYTES_MASK & value2.low);
    UINT32 fifth = value1.low * value2.high;
    UINT32 sixth = value2.low * value1.high;
    UINT32 seventh = value1.high * value2.high;
    tempSecond.high = 0;
    tempSecond.low = (LOW_TWO_BYTES_MASK & value1.low) * ((HIGH_TWO_BYTES_MASK & value2.low) >> 16);
    jlongShiftLeft(tempSecond, 16);
    tempThird.high = 0;
    tempThird.low = ((HIGH_TWO_BYTES_MASK & value1.low) >> 16) * (LOW_TWO_BYTES_MASK & value2.low);
    jlongShiftLeft(tempThird, 16);
    tempFourth.high = 0;
    tempFourth.low = ((HIGH_TWO_BYTES_MASK & value1.low) >> 16) * ((HIGH_TWO_BYTES_MASK & value2.low) >> 16);
    jlongShiftLeft(tempFourth, 32);
    
    result.low = first;
    result.high = fifth + sixth + seventh;
    jlongIncrement(result, tempSecond);
    jlongIncrement(result, tempThird);
    jlongIncrement(result, tempFourth);
    return result;
}

//TODO: jlongDivide
jlong jlongDivide(jlong value1, jlong value2)
{
    /* for now we just work on the low end */
    value1.high = 0;
    value1.low /= value2.low;
    return value1;
}
//TODO: jlongRemainder
jlong jlongRemainder(jlong value1, jlong value2)
{
    /* for now we just work on the low end */
    value1.high = 0;
    value1.low %= value2.low;
    return value1;
}

jlong jlongShiftLeft(jlong value, UINT32 shift)
{
    if(shift >= 0x20) {
        value.high = value.low << (shift - 0x20);
        value.low = 0;
    }
    else {
        value.high = (value.high << shift) | (value.low >> (0x20 - shift));
        value.low <<= shift;
    }
    return value;
}

jlong jlongShiftRight(jlong value, UINT32 shift)
{
    INT32 convertedHigh = (INT32) value.high;

    if(shift >= 0x20) {
        value.low = convertedHigh >> (shift - 0x20);
    }
    else {
        value.low = (value.low >> shift) | (convertedHigh << (0x20 - shift));
    }
    value.high = convertedHigh >> shift;
    return value;
}

jlong jlongUnsignedShiftRight(jlong value, UINT32 shift)
{
    if(shift >= 0x20) {
        value.low = value.high >> (shift - 0x20);
        value.high = 0;
    }
    else {
        value.low = (value.low >> shift) | (value.high << (0x20 - shift));
        value.high >>= shift;
    }
    return value;
}


#if IMPLEMENTS_FLOAT

#define exp32 0x100000000 /* 2^32 */
#define exp32float 0x100000000f

jfloat jlong2Float(jlong value)
{
    jfloat result = (jfloat) value.high;
    result *= exp32float;
    result += value.low;
    return result;
}

jdouble jlong2Double(jlong value)
{
    jdouble result = value.high;
    result *= exp32;
    result += value.low;
    return result;
}

jlong jdouble2Long(jdouble value)
{
    jlong result;
    result.high = (UINT32) (value / exp32);
    result.low = (UINT32) value;
    return result;
}

jlong jfloat2Long(jfloat value)
{
    jlong result;
    result.high = (UINT32) (value / exp32);
    result.low = (UINT32) value;
    return result;
}

#endif

#endif

/* note: in (x / y) or (x % y), x is the dividend while y is the divisor */

jfloat jfloatMod(jfloat dividend, jfloat divisor)
{
    return (jfloat) fmod((jdouble) dividend, (jdouble) divisor);
}

jdouble jdoubleMod(jdouble dividend, jdouble divisor)
{
    return fmod(dividend, divisor);
}

