package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 7:49:33 PM
 * Modified: April 10, 2002 7:49:33 PM
 */


public class DataInputStream extends InputStream implements DataInput
{
	protected InputStream in;	

	public final short readShort() throws IOException {
		int high = in.read();
		int low = in.read();
		if(low < 0) {
			throw new EOFException();
		}
		return (short) ((high << 8) | low);
	}
	
	public int readUnsignedShort() throws IOException {
		int high = in.read();
		int low = in.read();
		if(low < 0) {
			throw new EOFException();
		}
		return (high << 8) | low;
	}
	
	public final String readUTF() throws IOException {
		return readUTF(this);
	}
	
	public int read() throws IOException {
		return in.read();
	}
	
	public final int read(byte[] b) throws IOException {
		return in.read(b);
	}
	
	public final int read(byte[] b, int off, int len) throws IOException {
		return in.read(b, off, len);
	}
	
	public final void readFully(byte[] b) throws IOException {
		readFully(b, 0, b.length);
	}
	
	public final void readFully(byte[] b, int off, int len) throws IOException {
		int numRead;
		while(len > 0) {
			numRead = in.read(b, off, len);
			if(numRead < 0) {
				throw new EOFException();
			}
			off += numRead;
			len -= numRead;
		}
	}
	
	public final int skipBytes(int n) throws IOException {
		return (int) in.skip(n);
	}
	
	public long skip(long n) throws IOException {
		return in.skip(n);
	}
	
	public byte readByte() throws IOException {
		int val = in.read();
		if(val < 0) {
			throw new EOFException();
		}
		return (byte) val;
	}
	
	public final int readUnsignedByte() throws IOException {
		int val = in.read();
		if(val < 0) {
			throw new EOFException();
		}
		return val;
	}
	
	public final boolean readBoolean() throws IOException {
		int val = in.read();
		if(val == -1) {
			throw new EOFException();
		}
		return val == 0 ? false : true;
	}
	
	public final char readChar() throws IOException {
		int high = in.read();
		int low = in.read();
		if(low < 0) {
			throw new EOFException();
		}
		return (char) ((high << 8) | low);
	}
	
	public final int readInt() throws IOException {
		int high = in.read();
		int highmiddle = in.read();
		int lowmiddle = in.read();
		int low = in.read();
		if(low < 0) {
			throw new EOFException();
		}
		return (((((high << 8) | highmiddle) << 8) | lowmiddle) << 8) | low;
	}
	
	public final long readLong() throws IOException {
		int val1 = in.read();
		int val2 = in.read();
		int val3 = in.read();
		int val4 = in.read();
		int val5 = in.read();
		int val6 = in.read();
		int val7 = in.read();
		int val8 = in.read();
		if(val8 < 0) {
			throw new EOFException();
		}
		return ((((((((((((val1 << 8) | val2) << 8) | val3) << 8) | val4 << 8) | val5) << 8) | val6) << 8) | val7) << 8) | val8;
	}
	
	public static final String readUTF(DataInput in) throws IOException {
		int length = in.readUnsignedShort();
		StringBuffer buffer = new StringBuffer(length);
		byte firstByte, secondByte, thirdByte;
		
		for(int i=0; i<length; i++) {
			firstByte = in.readByte();
			if((firstByte & 0x80) == 0) {
				buffer.append((char) firstByte);
			}
			else {
				try {
					secondByte = in.readByte();
					if((firstByte & 0xe0) == 0xc0) {
						if((secondByte & 0xc0) != 0x80) {
							throw new UTFDataFormatException();
						}
						else {
							buffer.append(((firstByte & 0x1F) << 6) | (secondByte & 0x3F));
						}
					}
					else {
						thirdByte = in.readByte();
						if((firstByte & 0xf0) == 0xe0) {
							if((secondByte & 0xc0) == 0x80 && (thirdByte & 0xc0) == 0x80) {
								buffer.append(((firstByte & 0x0F) << 12) | ((secondByte & 0x3F) << 6) | (thirdByte & 0x3F));
							}
							else {
								throw new UTFDataFormatException();
							}
						}
						else {
							throw new UTFDataFormatException();
						}
					}
				}
				catch(EOFException e) {
					throw new UTFDataFormatException();
				}
			}
		}//end for
		return buffer.toString();
	}
}

