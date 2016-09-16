package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 4:19:34 AM
 * Modified: April 10, 2002 4:19:34 AM
 */


public abstract class Writer
{
	protected Object lock;
	private char oneCharArray[] = new char[1];
	char buffer[] = new char[64];
	
	protected Writer() {
		lock = this;
	}
	
	protected Writer(Object lock) {
		if (lock == null) {
            throw new NullPointerException();
        }
		this.lock = lock;
	}

	public void write(char[] cbuf) throws IOException {
		write(cbuf, 0, cbuf.length);
	}
	
	
	public void write(int c) throws IOException {
		synchronized(lock) {
			oneCharArray[0] = (char) c;
			write(oneCharArray, 0, 1); 
		}
	}
	
	public void write(String str) throws IOException {
		write(str, 0, str.length());
	}
	
	public void write(String str, int off, int len) throws IOException {
		char chars[];
		if(len >= 64) {
			chars = new char[len];
		}
		else {
			chars = this.buffer;
		}
		synchronized(lock) {
			str.getChars(off, off + len, chars, 0);
			write(chars, 0, len);
		}
	}
	
	public abstract void write(char[] cbuf, int off, int len) throws IOException;
	
	public abstract void close() throws IOException;
	
	public abstract void flush() throws IOException;
}

