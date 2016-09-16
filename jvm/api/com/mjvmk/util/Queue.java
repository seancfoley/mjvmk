package com.mjvmk.util;

/**
** A semi-priority ordered list.
** The highest priority item in the list is removed first,
** unless a priority ignore frequency n is set, in which case
** every nth item removed is removed in FIFO order.
**
** @version
** @author 	Sean Foley
** @see
** @see
** @since	JDK 1.2
**/
public interface Queue
{
	public void setPriorityIgnoreFrequency(int frequency);
	
	public boolean isEmpty();
	
	public QueueObject removeNextObject();
	
	public QueueObject get(int index);
	
	public void enqueueObject(QueueObject object);
	
	public int size();
	
	public void setObjectPriority(QueueObject object, Priority priority);
	
	/*
	 * returns the priority of the removed object
     */
	public Priority removeObjectPriority(QueueObject object);
	
	/*
	 * returns the priority of the removed object
	 */
	public Priority getObjectPriority(QueueObject object);

}

