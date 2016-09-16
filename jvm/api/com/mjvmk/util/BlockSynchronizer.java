package com.mjvmk.util;

import java.util.Hashtable;

/**
** Handles synchronization in a more analysable and controllable fashion than that offered by the JVM
**
** Use this class as follows:
** try {
**	synchronizer.registerThread(labelString);
**	...code to be synchronized...
** }
** finally {
** 	synchronizer.deregisterThread();
** }
**
**	All code synchronized as above using the same synchronizer object will be synchronized and executed in order by this class
**
** @version
** @author 	Sean Foley
** @see
** @see
** @since	JDK 1.2
**/
public class BlockSynchronizer
{
	private Queue queue = new PriorityQueue();
	
	private boolean loggingEnabled = false;
	
	
	private BlockInfo executingBlockInfo = null;
	private Object synchronizingObject;
	private static Object singletonSynchronizingObject = null;
	private static BlockSynchronizer instance = null;

	public BlockSynchronizer() {
		synchronizingObject = this;
	}
	
	public BlockSynchronizer(Object synchronizingObject) {
		if(synchronizingObject == null)
			throw new NullPointerException();
		this.synchronizingObject = synchronizingObject;
	}
	
	//the frequency by which priorities are ignored
	//set frequency to 1 in order to disable priorities
	//priorities will be ignored 1 out of every 'frequency' method calls
	public void setPriorityIgnoreFrequency(int frequency) {
		queue.setPriorityIgnoreFrequency(frequency);
	}

	protected String getThreadsState() {
		int i;
		StringBuffer result = new StringBuffer("Registered threads waiting to execute (" + queue.size() + ")" + (queue.size()>0 ? ": " : "; "));
		BlockInfo blockInfo;
		for(i=0; i<queue.size(); i++) {
			blockInfo = (BlockInfo) queue.get(i);
			result.append(blockInfo.toString() + ((i < queue.size()-1 ) ? ", " : "; "));
		}
		boolean threadIsExecuting = (executingBlockInfo != null);
		result.append("Registered threads currently executing (" + (threadIsExecuting ? "1): " : "0)"));
		if(threadIsExecuting) {
			result.append(executingBlockInfo.toString());
		}
		return result.toString();
	}
	
	
	
	public void setLoggingEnabled(boolean enabled) {
		loggingEnabled = enabled;
	}
	
	public void registerThread () {
		registerThread((String) null);
	}
	
	public void registerThread (String blockID) {
		registerThread(new BlockInfo(blockID, Thread.currentThread()));
	}
	
	public void registerThread (String blockID, Priority priority) {
		BlockInfo blockInfo = new BlockInfo(blockID, Thread.currentThread());
		blockInfo.setPriority(priority);
		registerThread(blockInfo);
	}
	
	private void registerThread(BlockInfo blockInfo) {
		synchronized (synchronizingObject) {
			if(loggingEnabled) {
				log("Registering thread " + blockInfo.thread + (blockInfo.blockID == null ? "" : " for code block labeled " + blockInfo.blockID) + "; " + getThreadsState());
			}
			if(executingBlockInfo == null) {
				//indicate the current thread will be executing
				executingBlockInfo = blockInfo;
			}
			else {
				//add current thread to the list of threads waiting to execute
				queue.enqueueObject(blockInfo);
				do {
					//when the current thread wakes up, only continue execution if it is next in line -
					//ie executingThreadInfo has been set to threadInfo by a thread that is deregistering 
					try {
						synchronizingObject.wait();
					}
					catch (InterruptedException e) {}
				} while(executingBlockInfo != blockInfo);
			}
		}
	}

	public void deregisterThread() {
		Thread currentThread = Thread.currentThread();
		synchronized (synchronizingObject) {
			if(!queue.isEmpty()) {
				//indicate which thread should go next by setting executingThreadInfo
				//this is where priority is assigned if applicable
				executingBlockInfo = (BlockInfo) queue.removeNextObject();

				//wake up any threads waiting to execute - they will then check to see which threadInfo has been put in executingThreadInfo
				synchronizingObject.notifyAll();
			}
			else {
				//no threads waiting to execute - set executingThreadInfo to indicate this
				executingBlockInfo = null;
			}
			if(loggingEnabled) {
				log("Deregistered thread " + currentThread + "; " + getThreadsState());
			}
		}
	}
	
	protected void log(String s) {
		//do nothing
		//override this method if you like
	}
	
	//for a higher priority choose a smaller int
	protected void setBlockPriority(String blockID, Priority priority) {
		//because the MethodSet object equals method uses only the method field,
		//this setting will work for all calls to the same method
		queue.setObjectPriority(new BlockInfo(blockID), priority);
	}	
	
	//remove priorities for blocks previously set, returns the priority the block was mapped to
	protected Priority removeBlockPriority(String blockID) {
		return queue.removeObjectPriority(new BlockInfo(blockID));
	}
	
	protected Priority getBlockPriority(String blockID) {
		return queue.getObjectPriority(new BlockInfo(blockID));
	}
	
	//holds info regarding a block of code, the block identified by a string ID that is assigned
	private static class BlockInfo extends QueueObject
	{
		protected String blockID; //the block's id
		protected Thread thread; //a thread designated to execute the block of code
		
		BlockInfo(String blockID) {
			this(blockID, null);
		}
		
		BlockInfo(String blockID, Thread thread) {
			this.blockID = blockID;
			this.thread = thread;
		}
		
		public String toString() {
			return ((thread == null ? "" : thread.toString())  + (blockID == null ? "" : ", code block label: " + blockID)); 
		}
		
		public boolean equals(Object object) {
			if(!(object instanceof BlockInfo))
				return false;
			if(blockID == null)
				return false;
			BlockInfo blockInfo = (BlockInfo) object;
			return (blockID.equals(blockInfo.blockID));
		}
	}
}

