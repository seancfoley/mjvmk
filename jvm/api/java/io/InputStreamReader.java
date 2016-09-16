package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 4:10:17 PM
 * Modified: April 10, 2002 4:10:17 PM
 */


public class InputStreamReader extends Reader
{
	private InputStream inputStream;
	
	public InputStreamReader(InputStream is) {
		if(is == null) {
			throw new NullPointerException();
		}
		inputStream = is;
	}
	
	public InputStreamReader(InputStream is, String enc) throws UnsupportedEncodingException {
		this(is);
	}
	
	public int read() throws IOException {
		return inputStream.read();
	}
	
	public int read(char[] cbuf, int off, int len) throws IOException {
		if(off < 0 || off + len > cbuf.length || len < 0) {
			throw new IndexOutOfBoundsException();
		}
		int in;
		int begin = off;
		int end = off + len;
		for(; off < end; off++) {
			in = inputStream.read();
			if(in < 0) {
				return off - begin;
			}
			cbuf[off] = (char) in;
		}
		return len;
	}
	
	public long skip(long n) throws IOException {
		return inputStream.skip(n);
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
	
	public void reset() throws IOException {
		throw new IOException();
	}
	
	public void close() throws IOException {
		inputStream.close();
	}
}

