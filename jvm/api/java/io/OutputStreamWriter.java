package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 4:43:28 AM
 * Modified: April 10, 2002 4:43:28 AM
 */


public class OutputStreamWriter extends Writer
{
	private OutputStream outputStream;
	
	public OutputStreamWriter(OutputStream os) {
		if(os == null) {
			throw new NullPointerException();
		}
		outputStream = os;
	}
	
	public OutputStreamWriter(OutputStream os, String enc) {
		this(os);
	}
	
	public void write(int c) throws IOException {
		outputStream.write(c);
	}
		
	public void write(char[] cbuf, int off, int len) throws IOException {
		if(off < 0 || off + len > cbuf.length || len < 0) {
			throw new IndexOutOfBoundsException();
		}
		int end = off + len;
		for(; off < end; off++) {
			outputStream.write(cbuf[off]);
		}
	}

	public void write(String str) throws IOException {
		int len = str.length();
		for(int i=0; i < len; i++) {
			outputStream.write(str.charAt(i));
		}
	}
	
	public void write(String str, int off, int len) throws IOException {
		if(off < 0 || off + len > str.length() || len < 0) {
			throw new IndexOutOfBoundsException();
		}
		int end = off + len;
		for(; off < end; off++) {
			outputStream.write(str.charAt(off));
		}
	}
	
	public void flush() throws IOException {
		outputStream.flush();
	}
					
	public void close() throws IOException {
		outputStream.close();
	}
}

