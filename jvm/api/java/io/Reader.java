package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 3:55:41 PM
 * Modified: April 10, 2002 3:55:41 PM
 */


public abstract class Reader
{
	protected Object lock;
	private char oneCharArray[] = new char[1];
	
	protected Reader() {
		lock = this;
	}
	
	protected Reader(Object lock) {
		if(lock == null) {
			throw new NullPointerException();
		}
		this.lock = lock;
	}
	
	public int read() throws IOException {
		int res;
		synchronized(lock) {
			read(oneCharArray, 0, 1); 
			res = oneCharArray[0];
		}
		return res;
	}
	
	public int read(char[] cbuf) throws IOException {
		return read(cbuf, 0, cbuf.length);
	}
	
	public abstract int read(char[] cbuf, int off, int len) throws IOException;

	private char buffer[];
	
	public long skip(long n) throws IOException {
		if(n < 0) {
			throw new IllegalArgumentException();
		}
		else if(n == 0) {
			return 0;
		}
		synchronized(lock) {
			if(buffer == null) {
				buffer = new char[64];
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
		}
		
		return n;
	}

	public boolean ready() throws IOException {
		return false;
	}
	
	public boolean markSupported() {
		return false;
	}
	
	public void mark(int readAheadLimit) throws IOException {
		throw new IOException();
	}
	
	public void reset() throws IOException{
		throw new IOException();
	}
	
	public abstract void close() throws IOException;
}

