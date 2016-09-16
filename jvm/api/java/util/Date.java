package java.util;

/*
 * Author: Sean Foley
 * Created: April 12, 2002 12:26:27 AM
 * Modified: April 12, 2002 12:26:27 AM
 */


public class Date
{
	private long date;
	
	public Date() {}
	
	public Date(long date) {
		this.date = date;
	}
	
	public long getTime() {
		return date;
	}

	public void setTime(long time) {
		date = time;
	}
	
	public int hashCode() {
		return (int) (date ^(date >>> 32));
	}
	
	public boolean equals(Object obj) {
		try {
			return date == ((Date) obj).date;
		}
		catch(ClassCastException e) {
			return false;
		}
	}
}

