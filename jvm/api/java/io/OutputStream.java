package java.io;

/*
 * Author: Sean Foley
 * Created: March 28, 2002 12:30:50 AM
 * Modified: March 28, 2002 12:30:50 AM
 */


public abstract class OutputStream
{
	public abstract void write(int b) throws IOException;
	
	public void write(byte[] b) throws IOException {
		int end = b.length;
		for(int off=0; off < end; off++) {
			write(b[off]);
		}
	}
	
	public void write(byte[] b, int off, int len) throws IOException {
		if(len < 0 || off < 0 || off + len > b.length) {
			throw new IndexOutOfBoundsException();
		}
		for(int end = len + off; off < end; off++) {
			write(b[off]);
		}
	}
	
	public void flush() throws IOException {}
	
	public void close() throws IOException {}
}

