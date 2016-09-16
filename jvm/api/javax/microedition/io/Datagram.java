package javax.microedition.io;

/*
 * Author: Sean Foley
 * Created: April 13, 2002 12:38:01 AM
 * Modified: April 13, 2002 12:38:01 AM
 */

import java.io.DataInput;
import java.io.DataOutput;

public interface Datagram extends DataInput, DataOutput
{
	String getAddress(); 
 	byte[] getData();
 	int getLength(); 
 	int getOffset(); 
 	void reset();
 	void setAddress(Datagram reference);
 	void setAddress(String addr); 
 	void setData(byte[] buffer, int offset, int len); 
 	void setLength(int len);
}

