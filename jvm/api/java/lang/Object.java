package java.lang;

/*
 * Author: Sean Foley
 * Created: March 27, 2002 5:25:30 PM
 * Modified: March 27, 2002 5:25:30 PM
 */

public class Object {

	public boolean equals(Object object) {
		return this == object;
	}
	
	public String toString() {
		return getClass().getName() + "@" + Integer.toHexString(hashCode());
	}
	
	public final native Class getClass();
	
	public native int hashCode();
	
	public final native void notify();
	
	public final native void notifyAll();
	
	public final native void wait() throws InterruptedException;
	
	public final native void wait(long timeout) throws InterruptedException;
	
	public final native void wait(long timeout, int nanoseconds) throws InterruptedException;

}
