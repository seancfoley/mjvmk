package java.lang;

/*
 * Author: Sean Foley
 * Created: March 27, 2002 11:03:46 PM
 * Modified: March 27, 2002 11:03:46 PM
 */


public class Thread implements Runnable
{
	/* Do not change these fields, they must match their counterparts in the VM */
	public static final int MAX_PRIORITY = 7;
	public static final int MIN_PRIORITY = 1;
	public static final int NORM_PRIORITY = 4;
	
	private Runnable runnable;
	private Object internalThreadObject;
	
	public Thread() {}
	
	public Thread(Runnable target) {
		this.runnable = target;
	}
	
	public static native Thread currentThread();
	
	public static native void yield();
	
	public static native void sleep(long millis) throws InterruptedException;
	
	public native void start();

	public void run() {
		if(runnable != null) {
			runnable.run();
		}
	}
	
	public native final boolean isAlive();
	
	public final native void setPriority(int newPriority);
	
	public final native int getPriority();
	
	public static native int activeCount();
	
	public native void interrupt();
	
	public final native void join() throws InterruptedException;
	
	public String toString() {
		return super.toString() + " priority:" + getPriority();
	}
}

