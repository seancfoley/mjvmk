package java.lang;

/*
 * Author: Sean Foley
 * Created: March 27, 2002 5:37:12 PM
 * Modified: March 27, 2002 5:37:12 PM
 */

import java.io.UnsupportedEncodingException;

public final class String
{
	/* Do not move these fields, they are altered internally by the VM */
	int length;
	char chars[];
	int offset;
	
	public String() {
		this(new char[0]);
	}
	
	public String(byte[] bytes) {
		int len = length = bytes.length;
		chars = new char[len];
		if(len > 0) {
			System.arraycopy(bytes, 0, chars, 0, length());
		}
	}

	public String(byte[] bytes, int offset, int len) {
		length = len;
		chars = new char[len];
		/* the array copy will throw any required exceptions */
		System.arraycopy(bytes, offset, chars, 0, len);
	}
	
	public String(byte[] bytes, String encoding) throws UnsupportedEncodingException {
		this(bytes);
	}
	
	public String(byte[] bytes, int offset, int len, String encoding) throws UnsupportedEncodingException {
		this(bytes, offset, len);
	}
	
	public String(char[] value) {
		int len = length = value.length;
		chars = new char[len];
		if(len > 0) {
			System.arraycopy(value, 0, chars, 0, len);
		}
	}
	
	public String(char[] value, int offset, int len) {
		length = len;
		chars = new char[len];
		/* the array copy will throw any required exceptions */
		System.arraycopy(value, offset, chars, 0, len);
	}
	
	public String(String value) {
		chars = value.chars;
		offset = value.offset;
		length = value.length();
	}

	public String(StringBuffer buffer) {
		this.chars = buffer.chars;
		this.length = buffer.length();
	}
	
	String(int offset, int length, char chars[]) {
		this.offset = offset;
		this.length = length;
		this.chars = chars;
	}
	
	/* The following three methods are native so that calls to these methods
	 * may be converted to VM instructions.
	 */
	public native int length();

	public native char charAt(int index);
	
	/* this native has no bounds checking, can cause VM crash if not called properly
	 * from within this package only.  However, it is quicker than a getfield of the 
	 * character array!
	 * Since Strings are immutable, we need not worry about inconsistencies caused by multiple threads.
	 */
	native char charAtInternal(int index);
	
	
	public void getChars(int srcBegin, int srcEnd, char[] dst, int dstBegin) {
		try {
			System.arraycopy(chars, offset + srcBegin, dst, dstBegin, srcEnd - srcBegin);
		}
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException();
		}
	}
	
	public byte[] getBytes(String enc) throws UnsupportedEncodingException {
		int len = length();
		byte bytes[] = new byte[len];
		try {
			System.arraycopy(chars, offset, bytes, 0, len);	
		}
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException();
		}
		return bytes;
	}
	
	/* Note that the quickened length() and internalCharAt() methods improve the speed of this method */
	public boolean equals(Object object) {
		try {
			String string = (String) object;
			int len = length();
			if(string.length() == len) {
				for(int i=0; i<len; i++) {
					//if(stringChars[stringOffset + i] != chars[offset + i]) {
					if(string.charAtInternal(i) != charAtInternal(i)) {
						return false;
					}
				}
				return true;
			}
		}
		catch(ClassCastException e) {}
		return false;
	}

	public int compareTo(String anotherString) {
		int lengthDiff = length() - anotherString.length();
		int compareLength = (lengthDiff < 0) ? length() : anotherString.length();
		for(int k=0; k<compareLength; k++) {
			int currentDiff = charAtInternal(k) - anotherString.charAtInternal(k);
			if(currentDiff != 0) {
				return currentDiff;
			}
		}
		return lengthDiff;
	}
	
	
	public boolean regionMatches(boolean ignoreCase, int toffset, String other,
                             int ooffset, int len) {
		if(toffset < 0 || ooffset < 0 || toffset + len > length() || ooffset + len > other.length()) {
			return false;
		}

		int caseInterval = Character.caseConverterInterval;
								
		for(int k=0; k<len; k++) {
			char c1 = charAtInternal(toffset + k);
			char c2 = other.charAtInternal(ooffset + k);
			if(c1 != c2) {
				if(ignoreCase) {
					if(c1 >= 'A' && c1 <= 'Z') {
						if(c1 + caseInterval != c2) {
							return false;
						}
					}
					else if(c1 >= 'a' && c1 <= 'z') {
						if(c2 + caseInterval != c1) {
							return false;
						}
					}
					else return false;
				}
				else return false;
			}
		}
		return true;
		
	}
	
	public boolean startsWith(String prefix, int toffset) {
		return regionMatches(false, toffset, prefix, 0, prefix.length());
	}

	public boolean startsWith(String prefix) {
		return regionMatches(false, 0, prefix, 0, prefix.length());
	}

	public boolean endsWith(String suffix) {
		int suffixLength = suffix.length();
		return regionMatches(false, length() - suffixLength, suffix, 0, suffixLength);
	}

	public int hashCode() {
		int hash = 0;
		int len = length();
		for(int i=0; i<len; i++) {
			hash += hash * 31 + charAtInternal(i);
		}
		return hash;
	}

	public int indexOf(int c) {
		return indexOf(c, 0);
	}
	
	public int indexOf(int c, int fromIndex) {
		int len = length();
		if(fromIndex < 0) {
			fromIndex = 0;
		}
		for(int i=fromIndex; i<len; i++) {
			if(charAtInternal(i) == c) {
				return i;
			}
		}
		return -1;
	}
	
	public int lastIndexOf(int c, int fromIndex) {
		int len = length();
		if(fromIndex >= len) {
			fromIndex = len - 1;
		}
		for(int i=fromIndex; i>=0; i--) {
			if(charAtInternal(i) == c) {
				return i;
			}
		}
		return -1;
	}
	
	public int lastIndexOf(int c) {
		return lastIndexOf(c, length() - 1);
	}
	
	public int indexOf(String s) {
		return indexOf(s, 0);
	}
	
	public int indexOf(String s, int fromIndex) {
		int len = length();
		int sLen = s.length();
		if(fromIndex < 0) {
			fromIndex = 0;
		}
		if(sLen == 0) {
			return fromIndex;
		}
		int firstChar = s.charAtInternal(0);
		firstLoop:
		for(int i=fromIndex; i + sLen <= len ; i++) {
			if(charAtInternal(i) == firstChar) {
				for(int j=1; j<sLen; j++) {
					if(charAtInternal(i+j) != s.charAtInternal(j)) {
						continue firstLoop;
					}
				}
				return i;
			}
		}
		return -1;
	}
	
	public String substring(int beginIndex) {
		return substring(beginIndex, length());
	}
	
	public String substring(int beginIndex, int endIndex) {
		int len = length();
		if(beginIndex == 0 && endIndex == len) {
			return this;
		}
		try {
			if(beginIndex >= 0 && endIndex - beginIndex >= 0 && endIndex <= len) {
				return new String(beginIndex, endIndex - beginIndex, chars);
			}
		}
		catch(ArrayIndexOutOfBoundsException e) {}
		throw new StringIndexOutOfBoundsException();
	}
	
	public String concat(String str) {
		int len = str.length();
		if(len == 0) {
			return this;
		}
		int thisLen = length();
		char newChars[] = new char[len + thisLen];
		if(thisLen > 0) {
			System.arraycopy(chars, offset, newChars, 0, thisLen);
		}
		System.arraycopy(str.chars, str.offset, newChars, thisLen, len);
		return new String(0, thisLen + len, newChars);
	}
	
	public String replace(char oldChar, char newChar) {
		if(oldChar == newChar) {
			return this;
		}
		int len = length();
		for(int i=0; i<len; i++) {
			if(charAtInternal(i) == oldChar) {
				char newChars[] = new char[len];
				System.arraycopy(chars, offset, newChars, 0, len);
				newChars[i] = newChar;
				for(int j=i+1; j<len; j++) {
					if(charAtInternal(j) == oldChar) {
						newChars[j] = newChar;
					}
				}
				return new String(0, len, newChars);
			}
		}
		return this;
	}
	
	public String toLowerCase() {
		int caseInterval = Character.caseConverterInterval;
		int len = length();
		int c;
		for(int i=0; i<len; i++) {
			c = charAtInternal(i);
			if(c >= 'A' && c <= 'Z') {
				char newChars[] = new char[len];
				System.arraycopy(chars, offset, newChars, 0, len);
				newChars[i] = (char) (c + caseInterval);
				for(int j=i+1; j<len; j++) {
					c = charAtInternal(j);
					if(c >= 'A' && c <= 'Z') {
						newChars[j] = (char) (c + caseInterval);
					}
				}
				return new String(0, len, newChars);
			}
		}
		return this;
	}
	
	public String toUpperCase() {
		int caseInterval = Character.caseConverterInterval;
		int len = length();
		int c;
		for(int i=0; i<len; i++) {
			c = charAtInternal(i);
			if(c >= 'a' && c <= 'z') {
				char newChars[] = new char[len];
				System.arraycopy(chars, offset, newChars, 0, len);
				newChars[i] = (char) (c - caseInterval);
				for(int j=i+1; j<len; j++) {
					c = charAtInternal(j);
					if(c >= 'a' && c <= 'z') {
						newChars[j] = (char) (c - caseInterval);
					}
				}
				return new String(0, len, newChars);
			}
		}
		return this;
	}
	
	public String trim() {
		int len = length();
		if(len == 0) {
			return this;
		}
		int i=0;
		while(charAtInternal(i) <= ' ') {
			i++;
		}
		int j=len-1;
		while(charAtInternal(j) <= ' ' && j>=i) {
			j--;
		}
		if(i==0 && j==len - 1) {
			return this;
		}
		return new String(offset + i, j-i+1, chars);
	}
	
	public String toString() {
		return this;
	}
	
	public char[] toCharArray() {
		int len = length();
		char newChars[] = new char[len];
		if(len > 0) {
			System.arraycopy(chars, offset, newChars, 0, len);
		}
		return newChars;
	}
	
	public static String valueOf(Object obj) {
		if(obj == null) {
			return "null";
		}
		return obj.toString();
	}
	
	public static String valueOf(char[] data) {
		return new String(data);
	}
	
	public static String valueOf(char[] data, int offset, int count) {
		return new String(data, offset, count);
	}
	
	public static String valueOf(boolean b) {
		return b ? "true" : "false";
	}
	
	public static String valueOf(char c) {
		char newChars[] = new char[1];
		newChars[0] = c;
		return new String(0, 1, newChars);
	}
	
	public static String valueOf(int i) {
		return Integer.toString(i);
	}
	
	public static String valueOf(long l) {
		return Long.toString(l);
	}
}
