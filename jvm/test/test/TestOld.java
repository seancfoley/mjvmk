package test;

import com.mjvmk.util.BlockSynchronizer;
import com.mjvmk.util.RunnableSynchronizer;
import com.mjvmk.util.RunnableSynchronizer.RunnableSet;

public class TestOld {
	//TODO: multianewarray test
	
	//TODO: synch method synchronization tests
	public static void mainx(String args[]) {
		//System.out.println("Hi");
		System.out.println("Hi");
		//System.out.println("a" + "b" + "c");
		int a = 167;
		int mod = a % 50;
		System.out.println("Bye");
		System.out.println(mod + " Addition: " + mod);
		System.out.flush();
		//toString(1, 10);
	}
	
	public static void mainy(String args[]) throws Exception {
		TestOld t = new TestOld();
		t.runTest();
	}
	
	public void runTest() {
		useUpStack(0);
		useUpStack(0);
		System.out.println("done");
	}
	
	private int limit = 200;
	
	private void useUpStack(int i) {
		System.out.println("frame is number " + i);
		if(i < limit) {
			useUpStack(i + 1);
		}
	}
	
	public static void mainRun(String args[]) throws Exception {
		TestOld t = new TestOld();
		t.runTest8();
	}
	
	public static void mainPlain(String args[]) throws Exception {
		int a = 3;
		Object o = new Object();
	}
	
	private static void handleThrowable(RunnableSet set) {
		if(set.throwable != null) {
			set.throwable.printStackTrace();
		}
	}
	
	public static void main(String args[]) throws Exception {
		System.out.println("Starting tests");
		final TestOld t = new TestOld();
		Object o = new Object();
		 /*
		synchronized(o) {
			try {
				o.wait(2000);
			} catch(InterruptedException e) {}
		}
		// */
		long startTime = System.currentTimeMillis();
		RunnableSynchronizer r = new RunnableSynchronizer();
		RunnableSet set;
		r.start();

		// /*
		System.out.println("test 1");
		set = r.executeRunnable(new Runnable() { public void run() {
			t.runTest();
		}});
		handleThrowable(set);
		System.out.println("test 2");
		set = r.executeRunnable(new Runnable() { public void run() {
			try {t.runTest2();} catch(Exception e) {}
		}});
		handleThrowable(set);
			// */
		set = r.executeRunnable(new Runnable() { public void run() {
			t.runTest3();
		}});
		handleThrowable(set);
		// */
		// /*
		set = r.executeRunnable(new Runnable() { public void run() {
			t.runTest4();
		}});
		handleThrowable(set);
		// */
		// */
		set = r.executeRunnable(new Runnable() { public void run() {
				t.runTest5();
		}});
		handleThrowable(set);
		// /*
		set = r.executeRunnable(new Runnable() { public void run() {
			t.runTest6();
		}});
		handleThrowable(set);
		// /*
		set = r.executeRunnable(new Runnable() { public void run() {
			t.runTest7();
		}});
		handleThrowable(set);
		set = r.executeRunnable(new Runnable() { public void run() {
			try {t.runTest8();} catch(Exception e) {}
		}});
		handleThrowable(set);
		// */
		System.out.println("execution time: " + (System.currentTimeMillis() - startTime));
		
	}
	
	public static void mainz(String args[]) throws Exception {
		final TestOld t = new TestOld();
		Object o = new Object();
		synchronized(o) {
			try {
				o.wait(3000);
			} catch(InterruptedException e) {}
		}
		t.runTest();
		t.runTest2();
		t.runTest3();
		t.runTest4();
		t.runTest5();
		t.runTest6();
		t.runTest7();
		t.runTest8();
		
	}
	
	void runTest8() throws Exception {
		System.out.println("1: " + (new Bla() instanceof Runnable));
		System.out.println("2: " + !(this instanceof Runnable));
		System.out.println("3: " + (this instanceof TestOld));
		System.out.println("4: " + (this instanceof Object));
		Object blax = new Bla() { public void x(){} };
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
		
	void runTest7() {
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
			System.out.println("current total is " + total); //should be 167
		
		}
		System.out.println("total is " + total); //should be 157
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
		System.out.println("total is " + total); //should be 1200
		
	}
	
	private String runbla(Runnable r) {
		return r.toString();
	}
	
	synchronized void runTest6() {
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
		Thread t1 = new Thread(r);
		Thread t2 = new Thread(r2);
		t1.start();
		t2.start();
		r.run();
	}
	
	synchronized void multipleSynchronizer(int i, boolean doWait) {
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
						wait();
					}
					else {
						notify();
					}
				}
				catch(InterruptedException e) {}
				System.out.println("jumping out");
			}
		}
	}
	
	void runTest4() {
		//Object bla1[][] = new X[3][4];
		
		//bla1[0] = new X[2];
		
		Object X[] = new X[3];
		boolean[][][] r = new boolean[2][3][4];
		r[1][2][2] = true;
		Object s = new long[3];
		System.out.println("done");
	}
	
	static class X {}
	boolean thread1Running = true;
	boolean thread2Running = true;
		
	void runTest5() {
		final Object bla = new Object();
		System.out.println("Starting test");
		long a =2;
		long b = a % 4;
		final BlockSynchronizer sync = new BlockSynchronizer();
		Thread t1 = new Thread() {
			int counter = 0;
			
			public void run() {
				System.out.println("starting 1");
				while(counter++ < 100) {
					try {
						sync.registerThread("1");
						for(int k = 0; k<10; k++) {
							System.out.println("in 1");
						}
					}
					finally {
						System.out.println("deregistering 1");
						sync.deregisterThread();
						System.out.println("finished deregistering 1");
					}
					
				}
				synchronized(bla) {
					thread1Running = false;
					bla.notify();
				}
				
			}
		};
		
		Thread t2 = new Thread() {
			int counter = 0;
			int r = 0;
			public void run() {
				System.out.println("starting 2");
				while(counter++ < 200) {
					try {
						sync.registerThread("2");
						r = r++ % 100;
						if(r == 0) {
							try {
								System.out.println("2 is sleeping " + r);
								Thread.sleep(10);
							} catch(InterruptedException e) {}
						}
						for(int k = 0; k<15; k++) {
							System.out.println("in 2");
						}
						
					}
					finally {
						System.out.println("deregistering 2");
						sync.deregisterThread();
						System.out.println("finished deregistering 2");
					}
				}
				synchronized(bla) {
					thread2Running = false;
					bla.notify();
				}
			}
		};
		
		t1.start();
		t2.start();
		while(thread1Running || thread2Running) {
			synchronized(bla) {
				try {
					bla.wait();
				} catch(InterruptedException e) {}
			}
		}
	}
	
	void runTest3() {
		Thread t1, t2;
		int i = 2;
		
		int a = i / 3;
		t1 = new Thread(new Bla2(1));
		t2 = new Thread(new Bla2(2));
		System.out.println("revving up the threads");
		t1.start();
		t2.start();
		new Bla2(3).run();
		System.out.println("Done test 3");
	}
	
	void runTest2() throws Exception {
		try {
			thrower(new Bla());
		}
		catch(RuntimeException t) {
			System.out.println("Caught the exception " + t);
		}
	}
	
	void thrower(Runnable bla) throws Exception {
		bla.run();
	}
	
	static class Bla implements Runnable {
		public void run() {
			throw new RuntimeException() {};
		}
	}
	
	static Object synchronizer = new Object();
	static Object synchronizer2 = new Object();
	
	static class Bla2 implements Runnable {
		int num;
		
		Bla2() {
			this(0);
		}
				
		Bla2(int num) {
			this.num = num;
		}
		
		public void run() {
			int counter = 0;
			while(counter++ < 11) {
				synchronized(synchronizer) {
					for(int i=0; i<3; i++)
						System.out.println("Running a " + num);
					synchronizer.notify();
					for(int i=0; i<3; i++)
						System.out.println("Running b " + num);
					try {
						System.out.println("About to wait a bit");
						synchronizer.wait(7000);
					} catch(InterruptedException e) {}
					for(int i=0; i<3; i++)
						System.out.println("Running c " + num);
				}

			}
			System.out.println(num + " is done");
		}
	}
	
	/*
	try {
				//TODO remove this
				com.mjvmk.StandardOutputStream os = new com.mjvmk.StandardOutputStream();
				System.out.println("printing digit");
				os.write(currentChar);
				os.write(result[currentPosition]);
				System.out.println("finished printing digit");
				
			} catch(java.io.IOException e) {}
	*/
	
	
}
