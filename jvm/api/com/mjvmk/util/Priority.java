package com.mjvmk.util;

/**
** Description of class
**
** @version
** @author 	Sean Foley
** @see
** @see
** @since	JDK 1.2
**/
public final class Priority extends Enum
{
	public static final EnumGroup enumGroup = new EnumGroup(Priority.class, true);
	public static final Priority HIGH_PRIORITY = new Priority(0, enumGroup);
	public static final Priority MEDIUM_PRIORITY = new Priority(5, enumGroup);
	public static final Priority LOW_PRIORITY = new Priority(10, enumGroup);
	public static final Priority DEFAULT_PRIORITY = MEDIUM_PRIORITY;
	
	Priority() {}
	
	public Priority(int value, EnumGroup enumGroup) {
		super(value, enumGroup);
	}

	/*
	 * priorities with lower integral values have higher priority
	 */
	public static Priority fromInt(int value) {
		return (Priority) enumGroup.fromInt(value);
	}
	
	public static Priority getDefaultPriority() {
		return DEFAULT_PRIORITY;
	}
	
	public static Priority getNextPriority(Priority p, boolean higher) {
		return fromInt(p.getValue() + (higher ? 1 : -1));
	}
}

