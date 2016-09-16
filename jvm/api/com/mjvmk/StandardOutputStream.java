package com.mjvmk;

/*
 * Author: Sean Foley
 * Created: April 19, 2002 2:14:21 PM
 * Modified: April 19, 2002 2:14:21 PM
 */

import java.io.OutputStream;
import java.io.IOException;

public class StandardOutputStream extends OutputStream
{
	public native void write(int b) throws IOException;
	
	public native void flush() throws IOException;

	public native void write(byte[] b) throws IOException;
	
	public native void write(byte[] b, int off, int len) throws IOException;

}

