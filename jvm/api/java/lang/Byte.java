package java.lang;

/*
 * Author: Sean Foley
 * Created: March 30, 2002 6:35:17 PM
 * Modified: March 30, 2002 6:35:17 PM
 */


public final class Byte
{
	private byte value;
	
	public static final byte MIN_VALUE = -128;
	public static final byte MAX_VALUE = 127;
	
	public Byte(byte value) {
		this.value = value;
	}
	
	public native byte byteValue();
	
	public int hashCode() {
		return byteValue();
	}
	
	public static byte parseByte(String s) throws NumberFormatException {
      return parseByte(s, 10);
    }
	
	public static byte parseByte(String s, int radix) throws NumberFormatException {
      int val = Integer.parseInt(s, radix);
      if (val < MIN_VALUE || val > MAX_VALUE)
          throw new NumberFormatException();
      return (byte) val;
    }
	
	public boolean equals(Object obj) {
		try {
			Byte b = (Byte) obj;
			return b.byteValue() == byteValue();
		} 
		catch(ClassCastException e) {
			return false;
		}
	}
	
	public String toString() {
		return Integer.toString(value);
	}

}

