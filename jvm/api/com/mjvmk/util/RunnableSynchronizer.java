package com.mjvmk.util;

//import java.util.List;
//import java.util.LinkedList;
//import java.util.Map;
//import java.util.SortedMap;
//import java.util.NoSuchElementException;
//import java.util.TreeMap;
//import java.lang.reflect.InvocationTargetException;
//import java.lang.reflect.Method;


/**
** Runs a single thread that handles all access to the indicated methods, in order
**
** Use as follows:
**
**	MethodSynchronizer.MethodSet methodSet;
**
**	methodSet = methodSynchronizer.executeMethod(
**			theObject, 
**			theClass.class.getDeclaredMethod("methodName", new Class[] {Arg1.class, Arg2.class, Primitive.TYPE}), 
**			new Object[] {arg1, arg2, primitive});
**
**	//or
**
**	methodSet = methodSynchronizer.executeStaticMethod(
**			theClass.class.getDeclaredMethod("methodName", new Class[] {Arg1.class, Arg2.class, Primitive.TYPE}), 
**			new Object[] {arg1, arg2, primitive}); 
**
**	methodSet.throwAnyThrowables();
**	//return value is contained in methodSet.returnValue
**
**	where:
**		theObject: the object which contains the method
**		theClass: the class which defines the method
**		methodName: the method's name
**		Arg1, Arg2: the class name of the first two arguments
**		Primitive: the class which represents the third primitive argument
**		arg1, arg2, primitive: the method arguments, two objects and one primitive type
**
**	Define zero length arrays for the case of no arguments
**
**	You can also set priorities for methods, or set a priority for any executeMethod call.
**	Priorities alter the next method call to be executed if several are waiting.
**
** @version
** @author 	Sean Foley
** @see
** @see
** @since	JDK 1.2
**/
public class RunnableSynchronizer 
{
	private Thread thread; //the thread that will execute all methods
	
	
	//flags to indicate logging, whether the thread has been stopped, and whether it has been suspended
	private boolean stopRunning = false;
	private boolean threadIsSuspended = false;
	private boolean loggingEnabled = false;
	private boolean throwThrowables = true;
	
	private Queue queue = new PriorityQueue();
	
	public RunnableSynchronizer() {
		thread = new Thread() {
					public void run () {
						RunnableSynchronizer.this.run();
					}
				};
	}
	
	public void setLoggingEnabled(boolean enabled) {
		loggingEnabled = enabled;
	}
	
	
	public void start() {
		stopRunning = false;
		try {
			thread.start();
		}
		catch(IllegalThreadStateException e) {} //thrown if the thread was already started
	}
	
	public void stop() {
		stopRunning = true;
		synchronized(this) {
			notify();
		}
	}
	
	public void suspend() {
		threadIsSuspended = true;
	}
	
	public void resume() {
		if(threadIsSuspended) {
			synchronized(this) {
				threadIsSuspended = false;
				notify();
			}
		}
	}
	
	public void run() {
		RunnableSet runnableSet;
		while(true) {
			synchronized (this) {
				//if there are no methods to be executed or the thread is suspended, wait
				while (!stopRunning && (threadIsSuspended || queue.isEmpty())) {
					try { wait(); } catch (InterruptedException e) {}
				}
				if(stopRunning)
					break;
				runnableSet = (RunnableSet) queue.removeNextObject();
				if(loggingEnabled) {
					log(getLogString(runnableSet));
				}
			}

			synchronized(runnableSet) {
				try {
					runnableSet.runnable.run();
				}
				catch (Throwable t) {
					runnableSet.throwable = t;
				}
				runnableSet.done = true;
				runnableSet.notify();
			}
		}
	}

	public void throwRuntimeExceptions(boolean b) {
		throwThrowables = b;
	}
	
	public RunnableSet executeRunnable(Runnable runnable) {
		RunnableSet runnableSet = new RunnableSet(runnable);
		executeRunnable(runnableSet);
		if(throwThrowables && runnableSet.throwable instanceof RuntimeException) {
			throw (RuntimeException) runnableSet.throwable;
		}
		return runnableSet;
	}
	
	public RunnableSet executeRunnable(Runnable runnable, Priority priority) {
		RunnableSet runnableSet = new RunnableSet(runnable);
		runnableSet.setPriority(priority);
		executeRunnable(runnableSet);
		if(throwThrowables && runnableSet.throwable instanceof RuntimeException) {
			throw (RuntimeException) runnableSet.throwable;
		}
		return runnableSet;
	}
	
	private void executeRunnable(RunnableSet runnableSet) {
		
		//wait until the thread is either waiting or executing some other method
		synchronized(this) {
		
			//add the runnable to the queue
			queue.enqueueObject(runnableSet);
			
			
			//notify the thread to execute the runnable
			if(!threadIsSuspended)
				notify();
		}
		
		//now wait until the execution is done
		//note that the method may already be completed execution so the synchronization on methodSet is necessary
		synchronized(runnableSet) {
			while(!runnableSet.done) {
				try { runnableSet.wait(); } catch(InterruptedException e) {} 
			}
		}
		
		return;
	}
	
	protected void log(String s) {
		//do nothing
		//override this method if you like
		//logging is already synchronized amongst threads, no further synchronization is necessary
	}
	
	private String getLogString(RunnableSet runnableSet) {
		StringBuffer logString;
		int queueSize = queue.size();
		if(queueSize == 0) {
			return "Executing " + runnableSet;
		}
		logString = new StringBuffer("Runnables waiting to execute:");
		for(int i=0; i<queueSize; i++) {
			logString.append(" ");
			logString.append(i+1);
			logString.append(") ");
			logString.append(queue.get(i).toString());
		}
		return logString.append("\nExecuting " + runnableSet).toString();
	}
	
	/* the frequency by which priorities are ignored
	 * set frequency to 1 in order to disable priorities
	 * priorities will be ignored 1 out of every 'frequency' method calls
     */
	public void setPriorityIgnoreFrequency(int frequency) {
		queue.setPriorityIgnoreFrequency(frequency);
	}

	
	
	/* for a higher priority choose a smaller int 
	 */
	protected void setRunnablePriority(Runnable runnable, Priority priority) {
		queue.setObjectPriority(new RunnableSet(runnable), priority);
	}	
	
	/* remove priorities for methods previously set, returns the priority the method was mapped to
	 */
	protected Priority removeMethodPriority(Runnable runnable) {
		return queue.removeObjectPriority(new RunnableSet(runnable));
	}
	
	protected Priority getMethodPriority(Runnable runnable) {
		return queue.getObjectPriority(new RunnableSet(runnable));
	}
	
	/* represents a routine to be run by the synchronizer */
	public class RunnableSet extends QueueObject
	{
		public Runnable runnable;
		public Throwable throwable;
		protected boolean done;
		
		protected RunnableSet(Runnable runnable) {
			this.runnable = runnable;
		}
		
		public void throwAnyThrowables() throws Throwable {
			if(throwable != null) {
				throw throwable;
			}
		}
		
		public String toString() {
			if(getPriority() == null)
				return runnable.toString();
			else
				return runnable + " priority: " + getPriority();
		}
		
		public boolean equals(Object object) {
			if(!(object instanceof RunnableSet))
				return false;
			RunnableSet runnableSet = (RunnableSet) object;
			return (runnableSet.runnable == this.runnable);
		}
	}

}


