package javax.microedition.io;

/*
 * Author: Sean Foley
 * Created: April 13, 2002 12:29:33 AM
 * Modified: April 13, 2002 12:29:33 AM
 */


public interface ContentConnection extends StreamConnection
{
	String getEncoding();
 	long getLength();
 	String getType();
}

