package com.mjvmk.util;

import java.util.Vector;

/**
** Description of class
**
** Note: When subclassing, you must override equals in order to customize how you map priority objects to priorities
** with PriorityList.setObjectPriority
**
** @version
** @author 	Sean Foley
** @see
** @see
** @since	JDK 1.2
**/


public class QueueObject
{
	private Vector equalPriorityObjects = null;
	private Priority priority = null;
	
	public QueueObject() {
		this(null);
	}
	
	public QueueObject(Priority priority) {
		this.priority = priority;
	}
	
	public boolean hasPriority() {
		return (priority != null);
	}
	
	public void setPriority(Priority priority) {
		this.priority = priority;
	}
	
	public Priority getPriority() {
		return priority;
	}
	
	QueueObject getNextPriorityObject() {
		QueueObject ret = (QueueObject) equalPriorityObjects.elementAt(0);
		equalPriorityObjects.removeElementAt(0);
		return ret;
	}

	void setEqualPriorityObjects(Vector equalPriorityObjects) {
		this.equalPriorityObjects = equalPriorityObjects;
	}
	
	void addEqualPriorityObject(QueueObject priorityObject) {
		if(equalPriorityObjects == null)
			equalPriorityObjects = new Vector();
		equalPriorityObjects.addElement(priorityObject);
	}
	
	Vector getEqualPriorityObjects() {
		return equalPriorityObjects;
	}
	
	boolean hasEqualPriorityObjects() {
		return (equalPriorityObjects != null && equalPriorityObjects.size() > 0);
	}
}

