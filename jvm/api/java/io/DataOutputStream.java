package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 6:16:50 AM
 * Modified: April 10, 2002 6:16:50 AM
 */


public class DataOutputStream extends OutputStream implements DataOutput
{
	protected OutputStream out;

	public DataOutputStream(OutputStream out) {
		if(out == null) {
			throw new NullPointerException();
		}
		this.out = out;
	}
	
	public void close() throws IOException {
		flush();
		out.close();
	}
	
	public void flush() throws IOException {
		out.flush();
	}
	
	public void write(byte[] b, int off, int len) throws IOException {
		out.write(b, off, len);
	}
	
	public void write(int b) throws IOException {
		out.write(b);
	}
	
	public final void writeBoolean(boolean v) throws IOException {
		out.write(v ? 1 : 0);
	}
	
	public final void writeByte(int v) throws IOException {
		out.write(v);
	}
	
	public final void writeShort(int v) throws IOException {
		out.write(v >> 8);
 		out.write(v);
	}
	
	public final void writeChar(int v) throws IOException {
		out.write(v >> 8);
 		out.write(v);
	}
	
	public final void writeInt(int v) throws IOException {
		out.write(v >> 24);
 		out.write(v >> 16);
 		out.write(v >> 8);
 		out.write(v);
	}
	
	public void writeLong(long v) throws IOException {
		out.write((int) (v >> 48));
 		out.write((int) (v >> 40));
 		out.write((int) (v >> 32));
 		out.write((int) (v >> 24));
 		out.write((int) (v >> 16));
 		out.write((int) (v >> 8));
 		out.write((int) v);
	}
	
	public void writeChars(String s) throws IOException {
		int len = s.length();
		for(int i=0; i<len; i++) {
			writeChar(s.charAt(i));
		}
	}
		
	public void writeUTF(String str) throws IOException {
		int len = str.length();
		int totalLen = 0;
		char c;
		for(int i=0; i<len; i++) {
			c = str.charAt(i);
			if(c <= '\u007f') {
				totalLen++;
			}
			else if(c < '\u07ff') {
				totalLen += 2;
			}
			else {
				totalLen += 3;
			}
		}
		if(totalLen > 65535) {
			throw new UTFDataFormatException();
		}
		writeShort(totalLen);
		for(int i=0; i<len; i++) {
			c = str.charAt(i);
			if(c <= '\u007f') {
				out.write(c);
			}
			else if(c < '\u07ff') {
				out.write(0xc0 | (0x1f & (c >> 6)));
				out.write(0x80 | (0x3f & c));
			}
			else {
				out.write(0xe0 | (0x0f & (c >> 12)));
				out.write(0x80 | (0x3f & (c >>  6)));
				out.write(0x80 | (0x3f & c));
			}
		}
	}
}

