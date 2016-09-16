package javax.microedition.io;

/*
 * Author: Sean Foley
 * Created: April 13, 2002 12:30:58 AM
 * Modified: April 13, 2002 12:30:58 AM
 */

import java.io.InputStream;
import java.io.DataInputStream;

public interface InputConnection extends Connection
{
	DataInputStream openDataInputStream();
 	InputStream openInputStream();
}

