package java.lang;

import java.io.InputStream;

/*
 * Author: Sean Foley
 * Created: March 27, 2002 5:38:48 PM
 * Modified: March 27, 2002 5:38:48 PM
 */


public final class Class
{
	/* Do not add any fields to the class, the VM internals
	 * are dependent upon its current structure 
	 */
	public static native Class forName(String className) throws ClassNotFoundException; 

	public native Object newInstance() throws InstantiationException, IllegalAccessException;

	public native String getName();
	
	public native InputStream getResourceAsStream(String name);

	public native boolean isInstance(Object o);
	
	public native boolean isAssignableFrom(Class c);
	
	public native boolean isInterface();
	
	public native boolean isArray();

	public String toString() {
		if(isInterface()) {
			return "interface " + getName();
		}
		return "class " + getName();
	}
	
}

