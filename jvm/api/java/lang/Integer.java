package java.lang;

/*
 * Author: Sean Foley
 * Created: March 27, 2002 6:41:15 PM
 * Modified: March 27, 2002 6:41:15 PM
 */


public class Integer
{
	private int value;
	
	public static final int MIN_VALUE = -2147483648;
		
	public static final int MAX_VALUE = 2147483647;
	
	public Integer(int value) {
		this.value = value;
	}
	
	public native int intValue();
	
	public int hashCode() {
		return intValue();
	}
	
	public byte byteValue() {
		return (byte) intValue();
	}
	
	public short shortValue() {
		return (short) intValue();
	}

	public long longValue() {
		return intValue();
	}
	
	public boolean equals(Object obj) {
		try {
			Integer i = (Integer) obj;
			return i.intValue() == intValue();
		} 
		catch(ClassCastException e) {
			return false;
		}
	}
	
	public static int parseInt(String s) throws NumberFormatException {
		return parseInt(s, 10);
	}
	
	//TODO: not sure if the following is adequate
	public static int parseInt(String s, int radix) throws NumberFormatException {
		if(s == null) {
			throw new NumberFormatException();
		}
		if(radix > Character.MAX_RADIX || radix < Character.MIN_RADIX) {
			throw new NumberFormatException("Radix outside allowable range");
		}
		int len = s.length();
		if(len == 0) {
			throw new NumberFormatException();
		}
		boolean negative;
		int firstIndex;
		if(s.charAtInternal(0) == '-') {
			negative = true;
			firstIndex = 1;
			if(len == 1) {
				throw new NumberFormatException();
			}
		}
		else {
			negative = false;
			firstIndex = 0;
		}
		int lastIndex = len;
		int result = 0;
		do {
			lastIndex--;
			int digit = Character.digit(s.charAtInternal(lastIndex), radix);
			if(digit < 0) {
				throw new NumberFormatException(s);
			}
			result = radix * result - digit;
			if(result > 0) {
				throw new NumberFormatException(s);
			}
		} while(lastIndex > firstIndex);
		if(!negative) {
			result = -result;
		}
		return result;
	}
	
	public static Integer valueOf(String s) throws NumberFormatException {
		return new Integer(Integer.parseInt(s, 10));
	}
	
	public static Integer valueOf(String s, int radix) throws NumberFormatException {
		return new Integer(Integer.parseInt(s, radix));
	}
	
	public String toString() {
		return toString(value, 10);
	}
	
	public static String toString(int i) {
		return toString(i, 10);
	}
	
	public static String toString(int i, int radix) {
		if(radix < Character.MIN_RADIX || radix > Character.MAX_RADIX)
            radix = 10;
		
		boolean negative = (i<0);
		int len = 11;
		int currentPosition = len;
		char result[] = new char[len];
		int currentDigit;
		char currentChar;

		do {
			currentPosition--;
			currentDigit = i % radix;
			if(currentDigit <= 9) {
				currentChar = (char) ('0' + currentDigit);
			}
			else {
				currentChar = (char) ('a' + (currentDigit - 10));
			}
			result[currentPosition] = currentChar;
			i /= radix;
		} while(i > 0);
		
		if(negative) {
			currentPosition--;
			result[currentPosition] = '-';
		}
		return new String(currentPosition, len - currentPosition, result);
	}
	
	private static String toUnsignedString(int i, int shift) {
		int radix = 1 << shift;
		int len = 11;
		char result[] = new char[len];
		int currentPosition = len;
		int currentDigit;
		char currentChar;
		int mask = radix - 1;
		do {
			currentPosition--;
			currentDigit = i & mask;
			if(currentDigit <= 9) {
				currentChar = (char) ('0' + currentDigit);
			}
			else {
				currentChar = (char) ('a' + (currentDigit - 10));
			}
			result[currentPosition] = currentChar;
			i >>>= shift;
		} while(i > 0);

		
		return new String(currentPosition, len - currentPosition, result);
	}
	
	public static String toBinaryString(int i) {
		return toUnsignedString(i, 1);
	}
	
	public static String toHexString(int i) {
		return toUnsignedString(i, 4);
	}

	public static String toOctalString(int i) {
		return toUnsignedString(i, 3);
	}

}

