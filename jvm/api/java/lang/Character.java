package java.lang;

/*
 * Author: Sean Foley
 * Created: April 5, 2002 4:12:53 PM
 * Modified: April 5, 2002 4:12:53 PM
 */


public class Character
{
	private char value;
	
	public static final char MIN_VALUE = '\u0000';
	public static final char MAX_VALUE = '\uffff';
	public static final int MAX_RADIX = 36;
	public static final int MIN_RADIX = 2;
	static char caseConverterInterval = 'a' - 'A';

	public Character(char value) {
		this.value = value;
	}
	
	public native char charValue();
	
	public int hashCode() {
		return charValue();
	}
	
	public String toString() {
		return String.valueOf(value);
	}
	
	public boolean equals(Object obj) {
		try {
			Character c = (Character) obj;
			return c.charValue() == charValue();
		} 
		catch(ClassCastException e) {
			return false;
		}
	}
	
	public static boolean isLowerCase(char ch) {
		return ch >= 'a' && ch <= 'z';
	}
	
	public static boolean isUpperCase(char ch) {
		return ch >= 'A' && ch <= 'Z';
	}
	
	public static boolean isDigit(char ch) {
		return ch >= '0' && ch <= '9';
	}
	
	public static char toLowerCase(char ch) {
		if(ch >= 'A' && ch <= 'Z') {
			return (char) (ch + caseConverterInterval);
		}
		return ch;
	}

	public static char toUpperCase(char ch) {
		if(ch >= 'a' && ch <= 'z') {
			return (char) (ch - caseConverterInterval);
		}
		return ch;
	}
	
	public static int digit(char ch, int radix) {
		if(ch > 'a') {
			ch -= 'a' - ('9' + 1);
		}
		else if(ch > 'A' ) {
			ch -= 'A' - ('9' + 1);
		}
		int result = ch - '0';
		return (result < radix) ? result : -1;
	}


	
}

