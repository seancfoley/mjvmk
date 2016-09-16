package com.mjvmk;

/*
 * Author: Sean Foley
 * Created: April 19, 2002 2:16:23 PM
 * Modified: April 19, 2002 2:16:23 PM
 */

import java.io.OutputStream;
import java.io.IOException;

public class StandardErrorStream extends OutputStream
{
	public native void write(int b) throws IOException;
}

