package javax.microedition.io;

/*
 * Author: Sean Foley
 * Created: April 13, 2002 12:43:31 AM
 * Modified: April 13, 2002 12:43:31 AM
 */

import java.io.IOException;

public class ConnectionNotFoundException extends IOException
{
	public ConnectionNotFoundException() {}

	public ConnectionNotFoundException(String s) {
		super(s);
	}
}

