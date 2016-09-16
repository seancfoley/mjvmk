package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 6:17:10 AM
 * Modified: April 10, 2002 6:17:10 AM
 */


public interface DataOutput
{
	void write(int b) throws IOException;
	void write(byte[] b) throws IOException;
	void write(byte[] b, int off, int len) throws IOException;		
	void writeBoolean(boolean v) throws IOException;
	void writeByte(int v) throws IOException;
	void writeShort(int v) throws IOException;
	void writeChar(int v) throws IOException;
	void writeInt(int v) throws IOException;
	void writeLong(long v) throws IOException;
	void writeChars(String s) throws IOException;
	void writeUTF(String str) throws IOException;
}

