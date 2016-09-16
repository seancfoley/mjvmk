package javax.microedition.io;

/*
 * Author: Sean Foley
 * Created: April 13, 2002 12:44:58 AM
 * Modified: April 13, 2002 12:44:58 AM
 */

import java.io.IOException;
import java.io.OutputStream;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.DataInputStream;

public class Connector
{
	static int READ = 1;
	static int WRITE = 2;
	static int READ_WRITE = 3;

	static Connection open(String name) throws IOException {
		return open(name, READ_WRITE, false);
	}
	
	static Connection open(String name, int mode) throws IOException {
		return open(name, mode, false);
	}
	
	static Connection open(String name, int mode, boolean timeouts) throws IOException {
		throw new ConnectionNotFoundException();
	}
	
	static DataInputStream openDataInputStream(String name) throws IOException {
		InputConnection connection = (InputConnection) Connector.open(name, Connector.READ);
        DataInputStream inputStream;
		try {
            inputStream = connection.openDataInputStream();
        } 
		finally {
            connection.close();
        }
		return inputStream;
	}
          
	static DataOutputStream openDataOutputStream(String name) throws IOException {
		OutputConnection connection = (OutputConnection) Connector.open(name, Connector.WRITE);
        DataOutputStream outputStream;
		try {
            outputStream = connection.openDataOutputStream();
        } 
		finally {
            connection.close();
        }
		return outputStream;
	}
          
	static InputStream openInputStream(String name) throws IOException {
		InputConnection connection = (InputConnection) Connector.open(name, Connector.READ);
        InputStream inputStream;
		try {
            inputStream = connection.openInputStream();
        } 
		finally {
            connection.close();
        }
		return inputStream;
	}
	
	static OutputStream openOutputStream(String name) throws IOException {
		OutputConnection connection = (OutputConnection) Connector.open(name, Connector.WRITE);
        OutputStream outputStream;
		try {
            outputStream = connection.openOutputStream();
        } 
		finally {
            connection.close();
        }
		return outputStream;
	}
}

