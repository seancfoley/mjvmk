package com.mjvmk;

/*
 * Author: Sean Foley
 * Created: March 27, 2002 5:25:30 PM
 * Modified: March 27, 2002 5:25:30 PM
 */


public class StackTrace {
	
	private StackTrace() {}
	
	/* Do not move these, they are altered internally by the VM */
	short[] stackTraceClassArray;
	int[] stackTraceMethodArray;
	StackTrace next;
}

