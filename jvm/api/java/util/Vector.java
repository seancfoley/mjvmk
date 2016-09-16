package java.util;

/*
 * Author: Sean Foley
 * Created: April 11, 2002 9:46:43 PM
 * Modified: April 11, 2002 9:46:43 PM
 */


public class Vector
{
	/* do not move these fields, they are accessed internally by the VM */
	protected int elementCount;
    protected Object[] elementData;
	protected int capacityIncrement;
    private int capacity;
	
	public Vector() {
		this(8);
	}
          
	public Vector(int initialCapacity) {
		this(initialCapacity, 16);
	}
           
	public Vector(int initialCapacity, int capacityIncrement) {
		if((initialCapacity | capacityIncrement) < 0) {
			throw new IllegalArgumentException();
		}
		this.capacityIncrement = capacityIncrement;
		this.capacity = initialCapacity;
		elementData = new Object[capacity];
	}
	
	public synchronized void copyInto(Object[] anArray) {
		System.arraycopy(elementData, 0, anArray, 0, elementCount);
	}

	public synchronized void trimToSize() {
		if(elementCount == capacity) {
			return;
		}
		Object newElementData[] = new Object[elementCount];
		System.arraycopy(elementData, 0, newElementData, 0, elementCount);
		elementData = newElementData;
		capacity = elementCount;
	}
	
	private void increaseCapacity(int minCapacity) {
		int newCapacity = capacity;
		do {
			newCapacity += capacityIncrement;
		} while(newCapacity < minCapacity);
		Object newElementData[] = new Object[newCapacity];
		System.arraycopy(elementData, 0, newElementData, 0, elementCount);
		elementData = newElementData;
		capacity = newCapacity;
	}
	
	public synchronized void ensureCapacity(int minCapacity) {
		if(capacity < minCapacity) {
			increaseCapacity(minCapacity);
		}
	}
	
	public synchronized void setSize(int newSize) {
		if(newSize > capacity) {
			increaseCapacity(newSize);
		}
		elementCount = newSize;
	}
	
	public int capacity() {
		return capacity;
	}
	
	public native int size();
	
	public boolean isEmpty() {
		return size() == 0;
	}
	
	public synchronized Object elementAt(int index) {
		if(index >= size()) {
			throw new ArrayIndexOutOfBoundsException();
		}
		return internalElementAt(index);
	}
	
	/* this method is converted to a bytecode for speed.  It can only
	 * be used internal to the util package.  There are several reasons why it cannot
	 * replace the elementAt method:
	 * - synchronization
	 * - cannot be public, because methods converted to bytecodes can never
	 * be public, because then they might be called as interface methods, which
	 * would change the instruction length from 3 to 5
	 * - bounds checking is ignored by the internal instruction for speed
	 */
	native Object internalElementAt(int index);
	
	public Enumeration elements() {
		return new Enumeration() {
				int index = 0;
				
				public boolean hasMoreElements() {
					return index < size();
				}
	           	
				public Object nextElement() {
					Object ret;
					synchronized(Vector.this) {
						if(index >= size()) {
							throw new NoSuchElementException();
						}
						ret = internalElementAt(index);
					}
					index++;
					return ret;
				}
			};
	}
	
	public boolean contains(Object elem) {
		return indexOf(elem, 0) != -1;
	}
	
	public int indexOf(Object elem) {
		return indexOf(elem, 0);
	}
	
	public synchronized int indexOf(Object elem, int index) {
		Object currentElement;
		int i;
		int size = size();
		if(elem == null) {
			for(i=index; i<size; i++) {
				if(null == internalElementAt(i)) {
					return i;
				}
			}
		}
		else {
			for(i=index; i<size; i++) {
				currentElement = internalElementAt(i);
				if(currentElement != null && elem.equals(currentElement)) {
					return i;
				}
			}
		}
		return -1;
	}
	
	public int lastIndexOf(Object elem) {
		return lastIndexOf(elem, size() - 1);
	}
	
	public synchronized int lastIndexOf(Object elem, int index) {
		Object currentElement;
		int i;
		if(elem == null) {
			for(i=index; i>=0; i--) {
				if(null == internalElementAt(i)) {
					return i;
				}
			}
		}
		else {
			for(i=index; i>=0; i--) {
				currentElement = internalElementAt(i);
				if(currentElement != null && elem.equals(currentElement)) {
					return i;
				}
			}
		}
		return -1;
	}
	
	public synchronized Object firstElement() {
		if(size() == 0) {
			throw new NoSuchElementException();
		}
		return internalElementAt(0);
	}
	
	public synchronized Object lastElement() {
		if(size() == 0) {
			throw new NoSuchElementException();
		}
		return internalElementAt(size() - 1);
	}
	
	public synchronized void setElementAt(Object obj, int index) {
		elementData[index] = obj;
	}
	
	public synchronized void removeElementAt(int index) {
		if(index < size() - 1) {
			System.arraycopy(elementData, index + 1, elementData, index, size() - index - 1); 
		}
		else if(index < 0 || index >= size()) { // handles the case where index == -1, size == 0 and index >= size()
			throw new ArrayIndexOutOfBoundsException();
		}
		if(size() > 0) {
			elementCount--;
		}
	}
	
	public void insertElementAt(Object obj, int index) {
		if(index <= size()) {
			if(size() == capacity) {
				increaseCapacity(size() + 1);
			}
			System.arraycopy(elementData, index, elementData, index + 1, size() - index); 
		}
		else if(index < 0 || index > size()) { // handles the case where index == -1, size == 0 and index >= size()
			throw new ArrayIndexOutOfBoundsException();
		}
		elementCount++;
	}
	
	public synchronized void addElement(Object obj) {
		if(size() == capacity) {
			increaseCapacity(size() + 1);
		}
		elementData[size()] = obj;
		elementCount++;
	}
	
	public synchronized boolean removeElement(Object obj) {
		int index = indexOf(obj, 0);
		if(index == -1) {
			return false;
		}
		System.arraycopy(elementData, index + 1, elementData, index, size() - index - 1); 
		elementCount--;
		return true;
	}
	
	public synchronized void removeAllElements() {
		int size = size();
		/* allow for garbage collection of the objects */
		for(int i=0; i<size; i++) {
			elementData[i] = null;
		}
		elementCount = 0;
	}
	
	public String toString() {
		StringBuffer buffer = new StringBuffer();
		Enumeration elements = elements();
		boolean first = true;
		while(elements.hasMoreElements()) {
			if(first) {
				first = false;
			}
			else {
				buffer.append(", ");
			}
			buffer.append(elements.nextElement());
		}
		return buffer.toString();
	}
}

