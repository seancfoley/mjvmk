package com.mjvmk.util;

import java.util.Vector;

/**
** All objects in this queue have equal priority
**
** @version
** @author 	Sean Foley
** @see
** @see
** @since	JDK 1.2
**/
public class ListQueue implements Queue
{
	private Vector list = new Vector();
	
	public void setPriorityIgnoreFrequency(int frequency) {}
	public void setObjectPriority(QueueObject object, Priority priority) {}
	public Priority removeObjectPriority(QueueObject object) { return Priority.getDefaultPriority(); }
	public Priority getObjectPriority(QueueObject object) { return Priority.getDefaultPriority(); }
	
	
	public boolean isEmpty() {
		return list.isEmpty();
	}
	
	public QueueObject removeNextObject() {
		QueueObject ret = (QueueObject) list.firstElement();
		list.removeElementAt(0);
		return ret;
	}
	
	public QueueObject get(int index) {
		return (QueueObject) list.elementAt(index);
	}
	
	public void enqueueObject(QueueObject object) {
		list.addElement(object);
	}
	
	public int size() {
		return list.size();
	}
}

