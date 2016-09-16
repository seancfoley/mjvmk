package java.util;

/*
 * Author: Sean Foley
 * Created: April 11, 2002 11:48:45 PM
 * Modified: April 11, 2002 11:48:45 PM
 */


public class Stack extends Vector
{
	public Stack() {}
	
	public Object push(Object item) {
		addElement(item);
		return item;
	}
	
	public synchronized Object pop() {
		int size = size();
		if(size == 0) {
			throw new EmptyStackException();
		}
		elementCount--;
		return internalElementAt(size);
	}
	
	public synchronized Object peek() {
		int size = size();
		if(size == 0) {
			throw new EmptyStackException();
		}
		return internalElementAt(size);
	}
	
	public boolean empty() {
		return size() == 0;
	}
	
	public synchronized int search(Object o) {
		int index = lastIndexOf(o);
		if(index == -1) {
			return -1;
		}
		return size() - index;
	}

}

