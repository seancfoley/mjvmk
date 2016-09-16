package java.util;

/*
 * Author: Sean Foley
 * 
 * This implementation of calendar ignores times before the epoch. 
 * (Jan 1, 1970 UTC)
 * Attempting to set the time before the epoch results will set the time
 * to the epoch.
 *
 * Created: April 12, 2002 3:03:10 AM
 * Modified: April 12, 2002 3:03:10 AM
 */


public abstract class Calendar
{
	public static final int YEAR = 0;
	public static final int MONTH = 1;
	public static final int DATE = 2;
	public static final int DAY_OF_MONTH = DATE;
	public static final int DAY_OF_WEEK = 3;
	public static final int AM_PM = 4;
	public static final int HOUR = 5;
	public static final int HOUR_OF_DAY = 6;
	public static final int MINUTE = 7;
	public static final int SECOND = 8;
	public static final int MILLISECOND = 9;
	
	public static final int SUNDAY = 0;
	public static final int MONDAY = 1;
	public static final int TUESDAY = 2;
	public static final int WEDNESDAY = 3;
	public static final int THURSDAY = 4;
	public static final int FRIDAY = 5;
	public static final int SATURDAY = 6;
	
	public static final int JANUARY = 0;
	public static final int FEBRUARY = 1;
	public static final int MARCH = 2;
	public static final int APRIL = 3;
	public static final int MAY = 4;
	public static final int JUNE = 5;
	public static final int JULY = 6;
	public static final int AUGUST = 7;
	public static final int SEPTEMBER = 8;
	public static final int OCTOBER = 9;
	public static final int NOVEMBER = 10;
	public static final int DECEMBER = 11;
		
	public static final int AM = 0;
	public static final int PM = 1;
	
	/* each value is 0-base, except dayOfMonth which is 1-base, and
	 * year which is 1970-based
	 */
	private int year;
	private int month;
	private int dayOfMonth;
	private int hourOfDay;
	private int minute;
	private int second;
	private int millisecond;
	
	private Date date;
	boolean dateNotCalculated;
	
	private TimeZone timeZone;
	
	private static int millisecondsPerSecond = 1000;
	private static int millisecondsPerMinute = 60 * millisecondsPerSecond;
	private static int millisecondsPerHour = 60 * millisecondsPerMinute;
	private static int millisecondsPerDay = 24 * millisecondsPerHour;
	private static int millisecondsPerYear = 365 * millisecondsPerDay;

	/* the following arrays have 13 entries to allow for calculation up to January in the next year */
	private static int millisecondsUpToMonth[] = new int[] {
		0,
		(31) * millisecondsPerDay,
		(31 + 28) * millisecondsPerDay,
		(31 + 28 + 31) * millisecondsPerDay,
		(31 + 28 + 31 + 30) * millisecondsPerDay,
		(31 + 28 + 31 + 30 + 31 ) * millisecondsPerDay,
		(31 + 28 + 31 + 30 + 31 + 30) * millisecondsPerDay,
		(31 + 28 + 31 + 30 + 31 + 30 + 31) * millisecondsPerDay,
		(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31) * millisecondsPerDay,
		(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30) * millisecondsPerDay,
		(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31) * millisecondsPerDay,
		(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30) * millisecondsPerDay,
		(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31) * millisecondsPerDay
	};
	
	private static int millisecondsUpToMonthInLeapYear[] = new int[] {
		0,
		millisecondsUpToMonth[1],
		millisecondsUpToMonth[2] + millisecondsPerDay,
		millisecondsUpToMonth[3] + millisecondsPerDay,
		millisecondsUpToMonth[4] + millisecondsPerDay,
		millisecondsUpToMonth[5] + millisecondsPerDay,
		millisecondsUpToMonth[6] + millisecondsPerDay,
		millisecondsUpToMonth[7] + millisecondsPerDay,
		millisecondsUpToMonth[8] + millisecondsPerDay,
		millisecondsUpToMonth[9] + millisecondsPerDay,
		millisecondsUpToMonth[10] + millisecondsPerDay,
		millisecondsUpToMonth[11] + millisecondsPerDay,
		millisecondsUpToMonth[12] + millisecondsPerDay,
	};

	
	private static Calendar thisInstance;
		
	protected Calendar() {
		date = new Date();
	}

	/* every multiple of 4 from 1972 is a leap year */
	private boolean isLeapYear(int year) {
		return (year - 1972) % 4 == 0;
	}
	
	/* leap years occuring before year */
	private int countLeapYears(int year) {
		/* every multiple of 4 from 1972 is a leap year */
		return (year - 1969) / 4;
	}
	
	/* calculate the date field from the other fields */
	private void calculateDate() {
		int value = 0;
		value += (year - 1970) * millisecondsPerYear;
		value += countLeapYears(year + ((month > 1) ? 1 : 0)) * millisecondsPerDay;
		value += millisecondsUpToMonth[month];
		value += millisecondsPerDay * (dayOfMonth - 1);
		value += millisecondsPerHour * hourOfDay;
		value += millisecondsPerMinute * minute;
		value += millisecondsPerSecond * second;
		value += millisecond;
		value = Math.max(value, 0);
		int offset = timeZone.getOffset(0, year, month, dayOfMonth, 
			getDayOfWeek(year, month, dayOfMonth), 
			hourOfDay * millisecondsPerHour + minute * millisecondsPerMinute + second * millisecondsPerSecond + millisecond);
		
		/* value is in local time, must adjust according to time zone */
		value -= offset;
		value = Math.max(value, 0);
		date.setTime(value);
		calculateFields();
		dateNotCalculated = false;
	}
	
	/* calculate the other fields based on the date field */
	private void calculateFields() {
		int time = (int) date.getTime();
		year = time / millisecondsPerYear + 1970;
		time %= millisecondsPerYear;
		time -= countLeapYears(year) * millisecondsPerDay;
		int monthArray[];
		if(isLeapYear(year)) {
			monthArray = millisecondsUpToMonthInLeapYear;
		}
		else {
			monthArray = millisecondsUpToMonth;
		}
		int i;
		for(i=1; i<monthArray.length && time >= monthArray[i]; i++);
		month = i - 1;
		time -= millisecondsUpToMonth[month];
		dayOfMonth = time / millisecondsPerDay + 1;	
		time %= millisecondsPerDay;
		
		/* adjust for the time zone */
		int offset = timeZone.getOffset(1, year, month, dayOfMonth, 
			getDayOfWeek(year, month, dayOfMonth), time);
		time += offset;
		if(time < 0) {
			dayOfMonth--;
			time += millisecondsPerDay;
		}
		else if(time > millisecondsPerDay) {
			dayOfMonth++;
			time -= millisecondsPerDay;
		}
		adjustForDayOutOfRange();
		
		/* now complete the calculation */
		hourOfDay = time / millisecondsPerHour;
		time %= millisecondsPerHour;
		minute = time / millisecondsPerMinute;
		time %= millisecondsPerMinute;
		second = time / millisecondsPerSecond;
		time %= millisecondsPerSecond;
		millisecond = time;
	}
	
	public synchronized final Date getTime() {
		if(dateNotCalculated) {
			calculateDate();
		}
		return date;
	}
	
	public static Calendar getInstance() {
		return getInstance(TimeZone.GMT);
	}
	
	public static Calendar getInstance(TimeZone zone) {
		if(thisInstance == null) {
			thisInstance = new Calendar(){};
		}
		thisInstance.timeZone = zone;
		return thisInstance;
	}
	
	protected synchronized long getTimeInMillis() {
		if(dateNotCalculated) {
			calculateDate();
		}
		return date.getTime();
	}
	
	protected synchronized void setTimeInMillis(long millis) {
		date.setTime(Math.max(millis, 0));
		calculateFields();
		dateNotCalculated = false;
	}
	
	private int getDayOfWeek(int year, int month, int dayOfMonth) {
		int daysSinceEpoch = 0;
		daysSinceEpoch = (year - 1970) * 365;
		daysSinceEpoch += countLeapYears(year);
		int monthArray[] = (isLeapYear(year) ? millisecondsUpToMonthInLeapYear : millisecondsUpToMonth);
		daysSinceEpoch += monthArray[month] / millisecondsPerDay;
		daysSinceEpoch += dayOfMonth - 1;
		/* TODO: this is correct only if the epoch was a Sunday,
		 adjust as necessary */
		return daysSinceEpoch % 7;
	}
	
	public synchronized final int get(int field) {
		if(dateNotCalculated) {
			calculateDate();
		}
		switch(field) {
			case YEAR:
				return year;
			case MONTH:
				return month;
			case DAY_OF_MONTH:
				return dayOfMonth;
			case DAY_OF_WEEK:
				return getDayOfWeek(year, month, dayOfMonth);
			case AM_PM:
				return (hourOfDay <= 11) ? AM : PM;
			case HOUR:
				return (hourOfDay <= 11) ? hourOfDay : hourOfDay - 12;
			case HOUR_OF_DAY:
				return hourOfDay;
			case MINUTE:
				return minute;
			case SECOND:
				return second;
			case MILLISECOND:
				return millisecond;
			default:
				throw new IllegalArgumentException();
		}
	}

	private int getDaysInMonth(int year, int month) {
		int monthArray[] = (isLeapYear(year) ? millisecondsUpToMonthInLeapYear : millisecondsUpToMonth);
		return (monthArray[month + 1] - monthArray[month]) / millisecondsPerDay;
	}

	private void adjustForDayOutOfRange() {
		if(dayOfMonth < 1) {
			if(month == 0) {
				month = 11;
				year--;
				dayOfMonth += getDaysInMonth(year, month);
			}
			else {
				month--;
				dayOfMonth += getDaysInMonth(year, month);
			}
		}
		else {
			int daysInMonth = getDaysInMonth(year, month);
			if(dayOfMonth > daysInMonth) {
				dayOfMonth -= daysInMonth;
				if(month == 11) {
					month = 0;
					year++;
				}
				else {
					month++;
				}
			}
		}
	}
	
	public synchronized final void set(int field, int value) {
		dateNotCalculated = true;
		switch(field) {
			case YEAR: 
				year = Math.max(1970, value);
				return;
			case MONTH:
				month = value;
				return;
			case DAY_OF_MONTH :
				dayOfMonth = value;
				return;
			case DAY_OF_WEEK: {
				int dayDifference = value - get(DAY_OF_WEEK);
				dayOfMonth += dayDifference;
				adjustForDayOutOfRange();
				return;
			}
			case AM_PM:
				if(get(AM_PM) != value) {
					if(value == PM) {
						hourOfDay += 12;
					}
					else if(value == AM) {
						hourOfDay -=12;
					}
					else {
						throw new IllegalArgumentException();
					}
				}
				return;
			case HOUR:
				if(get(AM_PM) == PM)
					value += 12;
				hourOfDay = value;
				return;
			case HOUR_OF_DAY:
				hourOfDay = value;
				return;
			case MINUTE:
				minute = value;
				return;
			case SECOND:
				second = value;
				return;
			case MILLISECOND:
				millisecond = value;
				return;
			default:
				throw new IllegalArgumentException();
		}
	}
	
	public boolean equals(Object obj) {
		try {
			Calendar calendar = (Calendar) obj;
			if(dateNotCalculated) {
				calculateDate();
			}
			return getTimeInMillis() == calendar.getTimeInMillis();
		}
		catch(ClassCastException e) {
			return false;
		}
	}
	
	public boolean before(Object when) {
		try {
			Calendar calendar = (Calendar) when;
			if(dateNotCalculated) {
				calculateDate();
			}
			return getTimeInMillis() < calendar.getTimeInMillis();
		}
		catch(ClassCastException e) {
			return false;
		}
	}
	
	public boolean after(Object when) {
		try {
			Calendar calendar = (Calendar) when;
			if(dateNotCalculated) {
				calculateDate();
			}
			return getTimeInMillis() > calendar.getTimeInMillis();
		}
		catch(ClassCastException e) {
			return false;
		}
	}
	
	public void setTimeZone(TimeZone value) {
		dateNotCalculated = true;
		timeZone = value;
	}
	
	public TimeZone getTimeZone() {
		return timeZone;
	}
	
}

