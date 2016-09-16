package java.lang;

/*
 * Author: Sean Foley
 * Created: March 28, 2002 12:21:03 AM
 * Modified: March 28, 2002 12:21:03 AM
 */


public class Runtime
{
	private static Runtime runtime;
	
	public static Runtime getRuntime() {
		if(runtime == null) {
			runtime = new Runtime();
		}
		return runtime;
	}
	
	public native void exit(int status);
	
	public native long freeMemory();
	
	public native long totalMemory();
	
	public native void gc();

}

