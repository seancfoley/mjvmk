package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 7:03:36 AM
 * Modified: April 10, 2002 7:03:36 AM
 */


public class ByteArrayOutputStream extends OutputStream
{
	protected byte[] buf;
	protected int count;
	private int bufferLength;
	private boolean closed;
	
	public ByteArrayOutputStream() {
		this(32);
	}
	
	public ByteArrayOutputStream(int size) {
		if(size < 0) {
			throw new IllegalArgumentException();
		}
		buf = new byte[size];
	}
	
	public void write(int b) {
		if(closed) {
			throw new RuntimeException();
		}
		if(count == bufferLength) {
			increaseCapacity(count + 1);
		}
		buf[count] = (byte) b;
		count++;
	}
	
	public void write(byte[] b, int off, int len) {
		if(closed) {
			throw new RuntimeException();
		}
		if(count + len > bufferLength) {
			increaseCapacity(count + len);
		}
		System.arraycopy(b, off, buf, count, len);
		count += len;
	}
	
	private void increaseCapacity(int minimumCapacity) {
		int newCapacity = 2 * (bufferLength + 1);
		if(newCapacity < minimumCapacity) {
			newCapacity = minimumCapacity;
		}
		byte newBytes[] = new byte[newCapacity];
		System.arraycopy(buf, 0, newBytes, 0, count);
		buf = newBytes;
		bufferLength = newCapacity;
	}
	
	public void reset() {
		if(closed) {
			throw new RuntimeException();
		}
		count = 0;
	}
	
	public byte[] toByteArray() {
		byte newBytes[] = new byte[count];
		System.arraycopy(buf, 0, newBytes, 0, count);
		return newBytes;
	}
	
	public int size() {
		return count;
	}
	
	public String toString() {
		return new String(buf, 0, count);
	}
	
	public void close() throws IOException {
		closed = true;
	}

}

