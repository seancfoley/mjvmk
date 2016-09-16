package java.lang;

/*
 * Author: Sean Foley
 * Created: March 30, 2002 6:49:11 PM
 * Modified: March 30, 2002 6:49:11 PM
 */


public final class Short
{
	private short value;
	
	public static final short MIN_VALUE = -32768;
	public static final short MAX_VALUE = 32767;
	
	public Short(short value) {
		this.value = value;
	}
	
	public native short shortValue();
	
	public int hashCode() {
		return shortValue();
	}
	
	public static short parseShort(String s) throws NumberFormatException {
      return parseShort(s, 10);
    }
	
	public static short parseShort(String s, int radix) throws NumberFormatException {
      int val = Integer.parseInt(s, radix);
      if (val < MIN_VALUE || val > MAX_VALUE)
          throw new NumberFormatException();
      return (short) val;
    }
	
	public boolean equals(Object obj) {
		try {
			Short b = (Short) obj;
			return b.shortValue() == shortValue();
		} 
		catch(ClassCastException e) {
			return false;
		}
	}
	
	public String toString() {
		return Integer.toString(value);
	}

}

