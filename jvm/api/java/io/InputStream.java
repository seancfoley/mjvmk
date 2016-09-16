package java.io;

/*
 * Author: Sean Foley
 * Created: March 27, 2002 6:23:27 PM
 * Modified: March 27, 2002 6:23:27 PM
 */


public abstract class InputStream
{
	public abstract int read() throws IOException;
	
	public int read(byte[] b) throws IOException {
		int len = b.length;
		if(len == 0) {
			return len;
		}
		int in = read();
		if(in < 0) {
			return -1;
		}
		int off = 0;
		b[off] = (byte) in;
		if(len == 1) {
			return 1;
		}
		int end = off + len;
		off++;
		try {
			do {
				 in = read();
			     if(in < 0) {
					return off;
				 }
				 b[off] = (byte) in;
				 off++;
			} while(off < end);
		}
		catch(IOException e) {
			return off;
		}
		return len;
	}
	
	public int read(byte[] b, int off, int len) throws IOException {
		if(b == null) {
			throw new NullPointerException();
		}
		if(len < 0 || off < 0 || off + len > b.length) {
			throw new IndexOutOfBoundsException();
		}
		if(len == 0) {
			return 0;
		}
		int in = read();
		if(in < 0) {
			return -1;
		}
		b[off] = (byte) in;
		if(len == 1) {
			return 1;
		}
		int end = off + len;
		off++;
		try {
			do {
				 in = read();
			     if(in < 0) {
					return off;
				 }
				 b[off] = (byte) in;
				 off++;
			} while(off < end);
		}
		catch(IOException e) {
			return off;
		}
		return len;
	}
	
	private byte buffer[];
	
	public long skip(long n) throws IOException {
		if(n <= 0) {
			return 0;
		}
		if(buffer == null) {
			buffer = new byte[64];
		}
		long leftToSkip = n;
		int toRead;
		int skipped;
		
		do {
			toRead = (int) Math.min(leftToSkip, 64);
			skipped = read(buffer, 0, toRead);
			if(skipped == 0) {
				return n - leftToSkip;
			}
			leftToSkip -= skipped;
		} while(leftToSkip > 0);
		return n;
	}
	
	public int available() throws IOException {
		return 0;
	}
	
	public void close() throws IOException {}
	
	public void mark(int readlimit) {}
	
	public void reset() throws IOException {
		throw new IOException();
	}
	
	public boolean markSupported() {
		return false;
	}
}

