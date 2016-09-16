package java.lang;

/*
 * Author: Sean Foley
 * Created: March 28, 2002 2:05:43 PM
 * Modified: March 28, 2002 2:05:43 PM
 */


public class ArrayIndexOutOfBoundsException extends IndexOutOfBoundsException
{
	public ArrayIndexOutOfBoundsException() {}
	
	public ArrayIndexOutOfBoundsException(int index) {
		super(Integer.toString(index));
	}
	
	public ArrayIndexOutOfBoundsException(String message) {
		super(message);
	}	

}

