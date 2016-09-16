package com.mjvmk;

import java.io.InputStream;
import java.io.IOException;

/*
 * Author: Sean Foley
 * Created: June 14, 2002 12:25:27 AM
 * Modified: June 14, 2002 12:25:27 AM
 */


final class ResourceInputStream extends InputStream
{
	/* Do not change these fields, they must match their counterparts in the VM */
	private Object resourceAccess;
	private boolean isClosed;
	
	private ResourceInputStream() {}
	
	public native int read() throws IOException;
	
	public native void close() throws IOException;
}

