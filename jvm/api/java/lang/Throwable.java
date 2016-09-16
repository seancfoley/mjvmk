package java.lang;

import com.mjvmk.StackTrace;

/*
 * Author: Sean Foley
 * Created: March 27, 2002 5:25:30 PM
 * Modified: March 27, 2002 5:25:30 PM
 */

public class Throwable
{
	/* Do not move these fields, they are altered internally by the VM */
	private String message;
	private StackTrace stackTrace;
	
	public Throwable() {
		createStackTrace();
	}
	
	public Throwable(String message) {
		createStackTrace();
		this.message = message;
	}
	
	public String getMessage() {
		return message;
	}
	
	public String toString() {
		if(message != null) {
			return getClass().getName() + ": " + message;
		}
		else {
			return getClass().getName();
		}
	}
	
	/* Note: stack traces are supported, but an option allows them to be 
	 * disabled in the VM.
	 */
	public native void printStackTrace();
	
	public native void createStackTrace();
}

