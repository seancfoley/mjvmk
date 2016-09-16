package java.lang;

/*
 * Author: Sean Foley
 * Created: March 28, 2002 1:44:39 PM
 * Modified: March 28, 2002 1:44:39 PM
 */


public class StringIndexOutOfBoundsException extends IndexOutOfBoundsException
{
	public StringIndexOutOfBoundsException() {}
	
	public StringIndexOutOfBoundsException(int index) {
		super(Integer.toString(index));
	}
	
	public StringIndexOutOfBoundsException(String message) {
		super(message);
	}
}

