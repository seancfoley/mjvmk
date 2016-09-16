package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 3:32:35 PM
 * Modified: April 10, 2002 3:32:35 PM
 */


public class ByteArrayInputStream extends InputStream
{
	protected  byte[] buf;
	protected  int count; 
	protected  int mark;
	protected  int pos;
 
	public ByteArrayInputStream(byte[] buf) {
		this(buf, 0, buf.length);
	}
	
	public ByteArrayInputStream(byte[] buf, int offset, int length) {
		if(buf == null) {
			throw new NullPointerException();
		}
		this.buf = buf;
		pos = offset;
		count = offset + length;
	}
	
	public int read() {
		if(pos < count) {
			return buf[pos++];
		}
		return -1;
	}
	
	public int read(byte[] b, int off, int len) {
		if(pos == count) {
			return -1;
		}
		int bytesToRead = Math.min(len, count - pos);
		if(bytesToRead > 0) {
			System.arraycopy(buf, pos, b, off, bytesToRead);
			pos += bytesToRead;
		}
		return bytesToRead;
	}
	
	public long skip(long n) {
		long bytesToSkip = Math.min(n, count - pos);
		pos += bytesToSkip;
		return bytesToSkip;
	}
	
	public int available() {
		return count - pos;
	}
	
	public boolean markSupported() {
		return true;
	}
	
	public void mark(int readAheadLimit) {
		this.mark = readAheadLimit;
	}

	public void reset() {
		pos = mark;
	}
	
	public void close() throws IOException {
		pos = count;
	}
}

