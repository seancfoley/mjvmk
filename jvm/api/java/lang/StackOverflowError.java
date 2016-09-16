package java.lang;

/*
 * Author: Sean Foley
 * Created: April 20, 2002 3:38:00 PM
 * Modified: April 20, 2002 3:38:00 PM
 */


public class StackOverflowError extends VirtualMachineError
{
	public StackOverflowError() {}

	public StackOverflowError(String s) {
		super(s);
	}

}

