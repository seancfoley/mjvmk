package java.util;

/*
 * Author: Sean Foley
 * Created: April 12, 2002 12:23:30 AM
 * Modified: April 12, 2002 12:23:30 AM
 */


public class Random
{
	private long seed;
	
	public Random() { 
		this(System.currentTimeMillis()); 
	}
	
	public Random(long seed) { 
		setSeed(seed); 
	}
	
	synchronized public void setSeed(long seed) {
       this.seed = (seed ^ 0x5DEECE66DL) & ((1L << 48) - 1);
 	}
	
	synchronized protected int next(int bits) {
       seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
       return (int)(seed >>> (48 - bits));
 	}
	
	public int nextInt() {  
		return next(32); 
	}
	
	public long nextLong() {
       return ((long)next(32) << 32) + next(32);
 	}
}

