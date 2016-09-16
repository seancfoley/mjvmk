package javax.microedition.io;

/*
 * Author: Sean Foley
 * Created: April 13, 2002 12:34:33 AM
 * Modified: April 13, 2002 12:34:33 AM
 */

import java.io.OutputStream;
import java.io.DataOutputStream;

public interface OutputConnection extends Connection
{
	DataOutputStream openDataOutputStream();
 	OutputStream openOutputStream(); 
}

