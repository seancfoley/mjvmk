package java.lang;

/*
 * Author: Sean Foley
 * Created: April 20, 2002 3:41:02 PM
 * Modified: April 20, 2002 3:41:02 PM
 */


public class ExceptionInInitializerError extends LinkageError
{
	private Throwable thrown;
	
	public ExceptionInInitializerError() {}

	public ExceptionInInitializerError(String s) {
		super(s);
	}
	
	public ExceptionInInitializerError(Throwable thrown) {
		this.thrown = thrown;
	}
	
	public void printStackTrace() {
		thrown.printStackTrace();
	}
	
	public Throwable getException() {
		return thrown;
	}
}

