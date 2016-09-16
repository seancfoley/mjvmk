package java.lang;

/*
 * Author: Sean Foley
 * Created: April 20, 2002 3:37:05 PM
 * Modified: April 20, 2002 3:37:05 PM
 */


public class OutOfMemoryError extends VirtualMachineError
{
	public OutOfMemoryError() {}

	public OutOfMemoryError(String s) {
		super(s);
	}
}

