package java.lang;

import java.io.PrintStream;
import com.mjvmk.StandardOutputStream;
import com.mjvmk.StandardErrorStream;

/*
 * Author: Sean Foley
 * Created: March 27, 2002 11:59:29 PM
 * Modified: March 27, 2002 11:59:29 PM
 */


public final class System
{
	public static final PrintStream out = new PrintStream(new StandardOutputStream());
	
	public static final PrintStream err = new PrintStream(new StandardErrorStream());
	
	public native static void arraycopy(Object src, int srcIndex, Object dest, int destIndex, int length);
	
	public native static long currentTimeMillis();

	public native static int identityHashCode(Object o);
	
	public native static String getProperty(String key);
	
	public static void exit(int status) {
		Runtime.getRuntime().exit(status);
	}
	
	public static void gc() {
		Runtime.getRuntime().gc();
	}
	
}

