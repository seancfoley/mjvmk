package java.lang;

/*
 * Author: Sean Foley
 * Created: April 6, 2002 3:25:35 AM
 * Modified: April 6, 2002 3:25:35 AM
 */


public class Long
{
	private long value;
	
	public static final long MIN_VALUE = 0x8000000000000000L;

    public static final long MAX_VALUE = 0x7fffffffffffffffL;
	
	public Long(long value) {
		this.value = value;
	}
	
	public long longValue() {
		return value;
	}
	
	public int hashCode() {
        return (int)(value ^ (value >>> 32));
    }
	
	public boolean equals(Object obj) {
		try {
			Long l = (Long) obj;
			return l.longValue() == longValue();
		} 
		catch(ClassCastException e) {
			return false;
		}
	}
	
	public String toString() {
		return toString(value, 10);
	}

	public static String toString(long l) {
		return toString(l, 10);
	}
	
	public static String toString(long l, int radix) {
		if(radix < Character.MIN_RADIX || radix > Character.MAX_RADIX)
            radix = 10;
		
		boolean negative = (l<0);
		int len = 11;
		int currentPosition = len;
		char result[] = new char[len];
		int currentDigit;
		char currentChar;
		
		do {
			currentPosition--;
			currentDigit = (int) (l % radix);
			if(currentDigit <= 9) {
				currentChar = (char) ('0' + currentDigit);
			}
			else {
				currentChar = (char) ('a' + (currentDigit - 10));
			}
			result[currentPosition] = currentChar;
			l /= radix;
		} while(l > 0);
		
		if(negative) {
			currentPosition--;
			result[currentPosition] = '-';
		}
		
		return new String(currentPosition, len - currentPosition, result);
	}
	
	public static long parseLong(String s) throws NumberFormatException {
		return parseLong(s, 10);
	}
	
	//TODO: not sure if the following is adequate, same goes for analogous method in Integer
	public static long parseLong(String s, int radix) throws NumberFormatException {
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
		long result = 0;
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
}

