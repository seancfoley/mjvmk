package java.lang;

/*
 * Author: Sean Foley
 * Created: May 9, 2002 4:24:24 PM
 * Modified: May 9, 2002 4:24:24 PM
 */


public class NoClassDefFoundError extends LinkageError
{
	public NoClassDefFoundError() {}


	public NoClassDefFoundError(String s) {
		super(s);
	}

}
