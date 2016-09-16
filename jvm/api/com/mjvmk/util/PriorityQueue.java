package com.mjvmk.util;

import java.util.Vector;
import java.util.Hashtable;
import java.util.NoSuchElementException;
import java.util.Enumeration;

/**
** Maintains a semi-FIFO list, by which calls to remove the next object in the list
** is dependent upon the priority of each member of the list.
**
** @version
** @author 	Sean Foley
** @see
** @see
** @since	JDK 1.2
**/
public class PriorityQueue implements Queue
{
	private int counter = 0;
	private int priorityCheckFrequency = 1;
	//private boolean usingPriorities = false;
	
	private Vector objectQueue = new Vector(); //the queue of methods to be executed
	
	//maps methods to priorities
	//if a priority isn't provided then check this map, if it isn't in the map then use default priority
	private Hashtable objectToPriorityMap = new Hashtable();
	
	//maintains a map of the first method set to be executed for each priority 
	private Hashtable priorityToObjectMap = new Hashtable();
	
	//the frequency by which priorities are ignored
	//set frequency to 1 in order to disable priorities
	//priorities will be ignored 1 out of every 'frequency' method calls
	public void setPriorityIgnoreFrequency(int frequency) {
		if(frequency < 1)
			throw new IllegalArgumentException("Cannot set frequency to less than 1");
		priorityCheckFrequency = frequency;
		if(frequency == 1) {
			//clear the priority map
			priorityToObjectMap.clear();
		}
	}
	
	//for a higher priority choose a smaller int
	public void setObjectPriority(QueueObject priorityObject, Priority priority) {
		objectToPriorityMap.put(priorityObject, priority);
	}	
	
	//remove priorities for methods previously set, returns the priority the method was mapped to
	public Priority removeObjectPriority(QueueObject priorityObject) {
		Priority priority = (Priority) objectToPriorityMap.remove(priorityObject);
		if(priority == null)
			priority = Priority.getDefaultPriority();
		return priority;
	}
	
	public Priority getObjectPriority(QueueObject priorityObject) {
		return (Priority) objectToPriorityMap.get(priorityObject);
	}
	
	public boolean isEmpty() {
		return objectQueue.isEmpty();
	}
	
	public int size() {
		return objectQueue.size();
	}

	public QueueObject get(int index) {
		return (QueueObject) objectQueue.elementAt(index);
	}
	
	public QueueObject removeNextObject() {
		QueueObject priorityObject;
		try {
			counter = ++counter % priorityCheckFrequency;
			if(counter == 0) {
				//don't bother with priorities this time through
				priorityObject = (QueueObject) objectQueue.firstElement();
				objectQueue.removeElementAt(0);
				if(priorityObject.hasPriority()) {
					
					//remove this object from the priority map
					priorityToObjectMap.remove(priorityObject.getPriority());
					
					//update the priorityMap
					priorityMapUpdate(priorityObject);
				}
				return priorityObject;
			}
			priorityObject = removeHighestPriorityObject();
			if(priorityObject == null) {
				QueueObject obj = (QueueObject) objectQueue.firstElement();
				objectQueue.removeElementAt(0);
				return obj;
			}
		}
		catch(IndexOutOfBoundsException e) {
			priorityObject = null;
		}
		return priorityObject;
	}
	
	//call this after removing priorityObject from the priority map
	private void priorityMapUpdate(QueueObject priorityObject) {
		if(priorityObject.hasEqualPriorityObjects()) {
			//put the next method set of the same priority into the priority map
			QueueObject nextObject = (QueueObject) priorityObject.getNextPriorityObject();
			nextObject.setEqualPriorityObjects(priorityObject.getEqualPriorityObjects());						
			priorityToObjectMap.put(priorityObject.getPriority(), nextObject);
		}
	}
	
	private QueueObject removeHighestPriorityObject() {
		Integer lowestPriorityInteger;
		Enumeration keys = priorityToObjectMap.keys();
		if(keys.hasMoreElements()) {
			lowestPriorityInteger = (Integer) keys.nextElement();
		}
		else {
			return null;
		}
		while(keys.hasMoreElements()) {
			Integer nextKey = (Integer) keys.nextElement();
			if(lowestPriorityInteger.intValue() > nextKey.intValue()) {
				lowestPriorityInteger = nextKey;
			}
		}

		QueueObject priorityObject = null;
		do {
			priorityObject = (QueueObject) priorityToObjectMap.remove(lowestPriorityInteger);
			
			/* make sure this object has not been removed already */
			if(!objectQueue.removeElement(priorityObject))
				priorityObject = null;
		} while(priorityObject != null);
		
		//update the priorityMap
		priorityMapUpdate(priorityObject);
		
		return priorityObject;
	}
	
	public void enqueueObject(QueueObject priorityObject) {
		objectQueue.addElement(priorityObject);
		if(!priorityObject.hasPriority()) {
			Priority priority = getObjectPriority(priorityObject);
			if(priority == null)
				priority = Priority.getDefaultPriority();
			priorityObject.setPriority(priority);
		}
		QueueObject otherPriorityObject = (QueueObject) priorityToObjectMap.get(priorityObject.getPriority());
		if(otherPriorityObject == null) {
			priorityToObjectMap.put(priorityObject.getPriority(), priorityObject);
		}
		else {
			otherPriorityObject.addEqualPriorityObject(priorityObject);
		}
	}
}

