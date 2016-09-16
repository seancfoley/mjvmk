package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 7:45:35 PM
 * Modified: April 10, 2002 7:45:35 PM
 */


public interface DataInput
{
	boolean readBoolean() throws IOException;     
 	byte readByte() throws IOException; 
 	char readChar() throws IOException; 
	void readFully(byte[] b) throws IOException;
	void readFully(byte[] b, int off, int len) throws IOException; 
	int readInt() throws IOException; 
	long readLong() throws IOException; 
	short readShort() throws IOException; 
	int readUnsignedByte() throws IOException;
	int readUnsignedShort() throws IOException;
	String readUTF() throws IOException; 
	int skipBytes(int n) throws IOException;
}

