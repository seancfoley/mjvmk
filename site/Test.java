package test;



public class Test {

	private void declareTest(String s) {
		try {
			Thread.sleep(1000);
			System.out.println("\nTesting " + s);
			Thread.sleep(5000);
		}
		catch(InterruptedException e) {}
	}

	public static void main(String args[]) throws Exception {
		System.out.println("Starting tests");
		final Test t = new Test();
		
		t.runStackGrowthTest();
		t.runTypeCheckTest();
		t.runSwitchTest();
		t.runThrowTest();
		t.runMultiArrayTest();
		t.runThreadTest();
		t.runThreadTest2();
		t.runThreadTest3();
		System.out.println("Tests complete");
	}
	
	/*
	 * Stack growth
	 */
	public void runStackGrowthTest() {
		declareTest("java stack growth");
		useUpStack(0);
		useUpStack(0);
	}
	
	private int limit = 200;
	
	private void useUpStack(int i) {
		System.out.println("frame is number " + i);
		if(i < limit) {
			useUpStack(i + 1);
		}
	}
	
	
	static class Bla3 extends Bla {
		public Bla3() {}
		
		public void x(){}
	}

	/* 
	 * type checking
	 */
	void runTypeCheckTest() throws Exception {
		declareTest("type checking");
		System.out.println("1: " + (new Bla() instanceof Runnable));
		System.out.println("2: " + !(this instanceof Runnable));
		System.out.println("3: " + (this instanceof Test));
		System.out.println("4: " + (this instanceof Object));
		Object blax = new Bla3();
		Bla blay = (Bla) blax.getClass().newInstance();
		System.out.println("5: " + (blax instanceof Bla));
		System.out.println("6: " + !(blax instanceof Bla2));
		System.out.println("7: " + (blay instanceof Object));
		System.out.println("8: " + (blay instanceof Bla));
		System.out.println("9: " + (blay instanceof Runnable));
		System.out.println("10: " + !(blay instanceof I));
		Object c = new Bla[3];
		System.out.println("11: " + (c instanceof Object));
		System.out.println("12: " + (c instanceof Runnable[]));
		System.out.println("13: " + (c instanceof Bla[]));
		System.out.println("14: " + !(c instanceof Bla));
	}
	
	static interface I {}
	
	/*
	 * Switch instructions
	 */
		
	void runSwitchTest() {
		declareTest("java switch instructions tableswitch and lookupswitch");
		int i;
		int switches[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
		for(i=0; i<switches.length; i++) {
			System.out.println("array index " + i + " has value " + switches[i]);
		}
		int total = 0;
		for(i=0; i<10; i++) {
			switch(switches[i]) {
			case 0:
			case 1:
				total += switches[i]; //0, 1
				break;
			case 2:
			case 3:
				total += 2 * switches[i];  //4, 6
				break;
			case 4:
			case 5:
				total += 3 * switches[i];//12, 15
				break;
			case 6:
			case 7:
			case 8:
				total += 4 * switches[i]; //24, 28, 32
				break;
			default:
				total += 5 * switches[i]; //45
			}
			System.out.println("current total is " + total);
		
		}
		System.out.println("total is " + total + ", should be 167"); //should be 167
		total = 0;
		for(i=0; i<500; i++) {
			switch(i) {
			case 300:
				total += 2 * i;
				System.out.println("current total is " + total);
				break;
			case 350:
			case 250:
				total += i;
				System.out.println("current total is " + total);
			}
		}
		System.out.println("total is " + total + ", should be 1200"); //should be 1200
		System.out.println("done");
	}
	
	
	/*
	 * Throwing, finally and interface method calling
	 */
	void runThrowTest() throws Exception {
		declareTest("throwing from interface method call, executing finally blocks");
		try {
			try {
				thrower(new Bla());
			}
			finally {
				System.out.println("in second finally block");
			}
		}
		catch(RuntimeException t) {
			System.out.println("Caught the exception " + t);
		}
		System.out.println("done");
	}
	
	static class Bla implements Runnable {
		public void run() {
			throw new RuntimeException() {};
		}
	}
	
	void thrower(Runnable bla) throws Exception {
		try {
			bla.run();
		}
		catch(Exception e) {
			System.out.println("caught exception " + e + "\nrethrowing");
			throw e;
		}
		finally {
			System.out.println("in first finally block");
		}
	}
	
	public int negIndex = -1;
	public int largeIndex = 3;
	
	/* 
	 * multi-dimensional array creation, access and assignment
	 */
	void runMultiArrayTest() {
		declareTest("multi-dimensional array creation, access and assignment");
		System.out.println("Creating, assigning to, and testing bounds of a 1-dimensional object array");
		Object xArray[] = new X[3];
		xArray[2] = new X();
		try {
			xArray[largeIndex] = xArray[2];
		}
		catch(ArrayIndexOutOfBoundsException e) {
			System.out.println("Caught " + e + " as expected");
		}
		try {
			xArray[negIndex] = xArray[0];
		}
		catch(ArrayIndexOutOfBoundsException e) {
			System.out.println("Caught " + e + " as expected");
		}
		
		System.out.println("Creating and assigning to a 3-dimensional primitive array");
		boolean[][][] r = new boolean[2][3][4];
		r[1][2][2] = true;
		
		System.out.println("Assigning a primitive array to an object reference");
		Object s = new long[3];
		
		System.out.println("done");
	}
	
	static class X {}
	
	/*
	 * thread sleep, synchronization, wait and notify
	 */
	synchronized void runThreadTest() {
		declareTest("thread synchronization, thread sleep, thread wait and thread notify");
		Runnable r = new Runnable() {
			public void run() {
				multipleSynchronizer(0, true);
				System.out.println("done " + Thread.currentThread());
			}
		};
		Runnable r2 = new Runnable() {
			public void run() {
				multipleSynchronizer(0, false);
				System.out.println("done " + Thread.currentThread());
			}
		};
		/* Thread t2 will wait until t1 and the main thread are waiting on the current
		 * object's monitor, and then notify them both
		 */
		Thread t1 = new Thread(r);
		Thread t2 = new Thread(r2);
		t1.start();
		t2.start();
		r.run();
		System.out.println("done");
	}
	
	int waitCount;
	int waitingThreads = 0;
	
	void multipleSynchronizer(int i, boolean doWait) {
		System.out.println("thread " + Thread.currentThread() + " synch: " + i);
		synchronized(this) {
			if(i < 7) {
				multipleSynchronizer(i+1, doWait);
			}
			else {
				try {
					if(doWait) {
						try {
							Thread.sleep(5000);
						}
						catch(InterruptedException e) {}
						waitingThreads++;
						System.out.println("thread " + Thread.currentThread() + " is waiting");
						wait();
					}
					else {
						while(true) {
							if(waitingThreads >= 2) {
								System.out.println("thread " + Thread.currentThread() + " is notifying all");
								notifyAll();
								break;
							}
							else {
								wait(1);
							}
						}
					}
				}
				catch(InterruptedException e) {}
				System.out.println("thread " + Thread.currentThread() + " jumping out");
			}
		}
	}

	
	/* 
	 * thread synchronization
	 */
	boolean thread1Running = true;
	boolean thread2Running = true;
		
	void runThreadTest2() {
		declareTest("thread synchronization and notification");
		
		final Object synchronizer = new Object();
		final Object synchronizer2 = new Object();
		
		/* Thread t1 and thread t2 will alternate the monitor entry
		 * for a while until thread t1 is done and thread t2 can finish 
		 * without any more delays
		 */
		Thread t1 = new Thread() {
			int counter = 0;
			
			public void run() {
				while(counter++ < 3) {
					synchronized(synchronizer) {
						for(int k = 0; k<4; k++) {
							System.out.println("in thread 1");
						}
					}
					Thread.yield();
				}
				synchronized(synchronizer2) {
					thread1Running = false;
					synchronizer2.notify();
				}
				
			}
		};
		
		Thread t2 = new Thread() {
			int counter = 0;
			public void run() {
				while(counter++ < 5) {
					synchronized(synchronizer) {
						for(int k = 0; k<3; k++) {
							System.out.println("in thread 2");
						}
					}
					Thread.yield();
				}
				synchronized(synchronizer2) {
					thread2Running = false;
					synchronizer2.notify();
				}
			}
		};
		
		t1.start();
		t2.start();
		
		/* the main thread will not continue until the two created threads have expired */
		while(thread1Running || thread2Running) {
			synchronized(synchronizer2) {
				if(thread1Running || thread2Running) {
					try {
						synchronizer2.wait();
					} catch(InterruptedException e) {}
				}
				else {
					break;
				}
			}
		}
		System.out.println("done");
	}
	
	

	/* 
	 * thread timed wait, synchronization and notification 
	 */
	void runThreadTest3() {
		declareTest("thread timed wait, synchronization and notification");
		Thread t1, t2;
		t1 = new Thread(new Bla2(1));
		t2 = new Thread(new Bla2(2));
		System.out.println("revving up the threads");
		
		t1.start();
		t2.start();
		new Bla2(3).run();
		while(deathCounter < 3) {
			doNothing();
		}
		System.out.println("done");
	}
	
	public void doNothing() {}

	static int deathCounter = 0;
	static Object jointSynchronizer = new String();
	
	static class Bla2 implements Runnable {
		int num;
		
		Bla2(int num) {
			this.num = num;
		}
		
		/*
		 * The three threads will alternate monitor entry.  The timed wait will not
		 * actually timeout until only a single thread is left running.
		 */
		
		public void run() {
			int counter = 0;
			while(counter++ < 11) {
				synchronized(jointSynchronizer) {
					for(int i=0; i<3; i++)
						System.out.println("Running " + num + " a ");
					jointSynchronizer.notify();
					for(int i=0; i<3; i++)
						System.out.println("Running " + num + " b ");
					try {
						System.out.println(num + " about to wait a bit");
						jointSynchronizer.wait(7000);
					} catch(InterruptedException e) {}
					for(int i=0; i<3; i++)
						System.out.println("Running " + num + " c ");
				}

			}
			deathCounter++;
			System.out.println(num + " is done");
		}
	}
	
	
}
