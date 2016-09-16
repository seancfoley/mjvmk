package java.lang;

/*
 * Author: Sean Foley
 * Created: March 30, 2002 6:05:46 PM
 * Modified: March 30, 2002 6:05:46 PM
 */


public final class Boolean
{
	private boolean value;

	public Boolean(boolean value) {
		this.value = value;
	}
	
	public native boolean booleanValue();
	
	public String toString() {
		return booleanValue() ? "true" : "false";
	}
	
	public int hashCode() {
        return value ? 1231 : 1237;
    }
	
	public boolean equals(Object obj) {
		try {
			Boolean b = (Boolean) obj;
			return b.booleanValue() == booleanValue();
		} 
		catch(ClassCastException e) {
			return false;
		}
	}
}

