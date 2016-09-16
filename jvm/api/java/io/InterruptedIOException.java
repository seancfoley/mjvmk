package java.io;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 4:41:10 PM
 * Modified: April 10, 2002 4:41:10 PM
 */


public class InterruptedIOException extends IOException
{
	public InterruptedIOException() {}

	public InterruptedIOException(String s) {
		super(s);
	}

}

