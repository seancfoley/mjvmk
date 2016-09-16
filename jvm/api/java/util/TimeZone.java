package java.util;

/*
 * Author: Sean Foley
 * Created: April 12, 2002 12:08:44 AM
 * Modified: April 12, 2002 12:08:44 AM
 */


public abstract class TimeZone
{
	private String ID;
	static TimeZone GMT = new GMTTimeZone();
	private static String[] IDs = {GMT.ID};
	
	public TimeZone() {}
	
	public abstract int getOffset(int era,
                              int year,
                              int month,
                              int day,
                              int dayOfWeek,
                              int millis);
	
	public abstract int getRawOffset();
	
	public abstract boolean useDaylightTime();
	
	public String getID() {
		return ID;
	}
	
	public static TimeZone getTimeZone(String ID) {
		return GMT;
	}

	public static TimeZone getDefault() {
		return GMT;
	}
	
	public static String[] getAvailableIDs() {
		return IDs;
	}

	private static class GMTTimeZone extends TimeZone {
		
		private GMTTimeZone() {
			super.ID = "GMT";
		}
		
		public int getOffset(int era,
                              int year,
                              int month,
                              int day,
                              int dayOfWeek,
                              int millis) {
								return 0;
		}
		
		public int getRawOffset() {
			return 0;
		}
		
		public boolean useDaylightTime() {
			return false;
		}
	}
}

