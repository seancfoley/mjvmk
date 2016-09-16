package java.lang;

/*
 * Author: Sean Foley
 * Created: March 27, 2002 6:57:15 PM
 * Modified: March 27, 2002 6:57:15 PM
 */


public final class StringBuffer
{
	/* Do not move these fields, they are altered internally by the VM */
	int length;
	char chars[];
	int bufferLength;
	boolean locked;
	
	public StringBuffer() {
		this(16);
	}
	
	public StringBuffer(int length) {
		chars = new char[length];
		bufferLength = length;
	}
		
	public StringBuffer(String str) {
		int len = str.length();
		int bl = len + 16;
		chars = new char[bl];
		bufferLength = bl;
		System.arraycopy(str.chars, str.offset, chars, 0, len);
		length = len;
	}
	
	/* the length() method is converted to an actual VM instruction */
	public native int length();

	public int capacity() {
		return bufferLength;
	}
	
	private void release() {
		int len = bufferLength;
		char newChars[] = new char[bufferLength];
		System.arraycopy(chars, 0, newChars, 0, length());
		chars = newChars;
		locked = false;
	}
	
	public synchronized void ensureCapacity(int minimumCapacity) {
		if(bufferLength >= minimumCapacity) {
			return;
		}
		increaseCapacity(minimumCapacity);
	}

	private void increaseCapacity(int minimumCapacity) {
		int newCapacity = 2 * (bufferLength + 1);
		if(newCapacity < minimumCapacity) {
			newCapacity = minimumCapacity;
		}
		char newChars[] = new char[newCapacity];
		if(chars == null) {
			System.out.println("chars is null");
		}
		System.arraycopy(chars, 0, newChars, 0, length());
		locked = false;
		chars = newChars;
		bufferLength = newCapacity;
	}
	
	public synchronized void setLength(int newLength) {
		int len = length();
		if(newLength <= len) {
			if(newLength < 0) {
				throw new StringIndexOutOfBoundsException(newLength);
			}
			length = newLength;
			return;
		}
		if(bufferLength < newLength) {
			increaseCapacity(newLength);
		}
		else if(locked) {
			release();
		}
		for(int i=0; i < newLength; i++) {
			chars[i] = '\0';
		}
		length = newLength;
	}
	
	public synchronized char charAt(int index) {
		try {
			return chars[index];
		}
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException();
		}
	}
	
	public synchronized void getChars(int srcBegin, int srcEnd, char[] dst, int dstBegin) {
		try {
			System.arraycopy(chars, srcBegin, dst, dstBegin, srcEnd - srcBegin);
		}
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException();
		}
	}
	
	public synchronized void setCharAt(int index, char ch) {
		if(locked) {
			release();
		}
		try {
			chars[index] = ch;
		}
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException();
		}
	}
	
	public StringBuffer append(Object obj) {
		return append(obj.toString());
	}
	
	public synchronized StringBuffer append(String str) {
		if (str == null) {
            str = String.valueOf(str);
        }
		int strLength = str.length();
		int thisLength = length();
		int totalLength = strLength + thisLength;
		if(bufferLength < totalLength) {
			increaseCapacity(totalLength);
		}
		System.arraycopy(str.chars, str.offset, chars, thisLength, strLength);
		length = totalLength;
		return this;
	}

	public StringBuffer append(char[] str) {
		return append(str, 0, str.length);
	}
	
	public synchronized StringBuffer append(char[] str, int off, int len) {
		int thisLength = length();
		int totalLength = len + thisLength;
		if(bufferLength < totalLength) {
			increaseCapacity(len + thisLength);
		}
		try {
			System.arraycopy(str, off, chars, thisLength, len);
			length = totalLength;
		}
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException();
		}
		return this;
	}
	
	public StringBuffer append(boolean b) {
		return append(String.valueOf(b));
	}
		
	public synchronized StringBuffer append(char c) {
		int thisLength = ++length;
		if(bufferLength < thisLength) {
			increaseCapacity(thisLength);
		}
		chars[thisLength - 1] = c;
		length = thisLength;
		return this;
	}
	
	public StringBuffer append(int i) {
		return append(Integer.toString(i, 10));
	}
	
	public StringBuffer append(long l) {
		return append(Long.toString(l));
	}
	
	public synchronized StringBuffer delete(int start, int end) {
		if(locked) {
			release();
		}
		int len = length();
		if(end > len) {
			end = len;
		}
		try {
			System.arraycopy(chars, end, chars, start, len - end);
			length -= end - start;
		} 
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException();
		}
		return this;
	}
	
	public synchronized StringBuffer deleteCharAt(int index) {
		if(locked) {
			release();
		}
		try {
			System.arraycopy(chars, index + 1, chars, index, length() - index - 1);
			length--;
		} 
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException(index);
		}
		return this;
	}
	
	public StringBuffer insert(int offset, Object obj) {
        return insert(offset, String.valueOf(obj));
    }
	
	public synchronized StringBuffer insert(int off, String str) {
		if(str == null) {
			str = String.valueOf(str);
		}
		int thisLength = length();
		int strLength = str.length();
		int totalLength = thisLength + strLength;
		if(bufferLength < totalLength) {
			increaseCapacity(totalLength);
		}
		else if(locked) {
			release();
		}
		try {
			System.arraycopy(chars, off, chars, off + strLength, thisLength - off);
			System.arraycopy(str.chars, str.offset, chars, off, strLength);
			length = totalLength;
		}
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException();
		}
		return this;
	}
	
	public StringBuffer insert(int off, char[] str) {
		int thisLength = length();
		int strLength = str.length;
		int totalLength = thisLength + strLength;
		if(bufferLength < totalLength) {
			increaseCapacity(totalLength);
		}
		else if(locked) {
			release();
		}
		try {
			System.arraycopy(chars, off, chars, off + strLength, thisLength - off);
			System.arraycopy(str, 0, chars, off, strLength);
			length = totalLength;
		}
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException();
		}
		return this;
	}

	public StringBuffer insert(int off, boolean b) {
		return insert(off, String.valueOf(b));
	}

	public StringBuffer insert(int off, char c) {
		int thisLength = length();
		int totalLength = thisLength + 1;
		if(bufferLength < totalLength) {
			increaseCapacity(totalLength);
		}
		else if(locked) {
			release();
		}
		try {
			System.arraycopy(chars, off, chars, off + 1, thisLength - off);
			chars[off] = c;
			length++;
		}
		catch(ArrayIndexOutOfBoundsException e) {
			throw new StringIndexOutOfBoundsException();
		}
		return this;
	}

	public StringBuffer insert(int off, int i) {
		return insert(off, Integer.toString(i));
	}

	public StringBuffer insert(int off, long l) {
		return insert(off, Long.toString(l));
	}

	public synchronized StringBuffer reverse() {
		if (locked) {
			release();
		}
		int len = length();
		char c;
		int j = (len + 1) >> 1;
		int i = (len - 2) >> 1;
        for (; i >= 0; ++j, --i) {
            c = chars[i];
            chars[i] = chars[j];
            chars[j] = c;
        }
		return this;
	}

	public synchronized String toString() {
		locked = true;
		return new String(this);
	}	
}

