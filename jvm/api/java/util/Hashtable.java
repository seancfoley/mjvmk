package java.util;

/*
 * Author: Sean Foley
 * Created: April 10, 2002 11:48:15 PM
 * Modified: April 10, 2002 11:48:15 PM
 */


public class Hashtable
{
	/* do not move these fields, they are accessed internally by the VM */
	private int size;
	private int loadFactor = 75; /* can range from 0 to 100 */	
	private int tableCapacity;
	private Entry[] entries;

	public Hashtable() {
		this(32);
	}
	
	public Hashtable(int initialCapacity) {
		if(initialCapacity < 0) {
            throw new IllegalArgumentException();
        }
		tableCapacity = initialCapacity;
		entries = new Entry[tableCapacity];
	}

	protected void rehash() {
		int oldCapacity = tableCapacity;
		tableCapacity <<= 1;
		Entry newEntries[] = new Entry[tableCapacity];
		Entry localEntries[] = entries;
		Entry currentEntry;
		Entry nextEntry;
		int hashIndex;
		for(int i=0; i<oldCapacity; i++) {
			currentEntry = localEntries[i];
			while(currentEntry != null) {
				nextEntry = currentEntry.next;
				hashIndex = currentEntry.hash % tableCapacity;
				currentEntry.next = newEntries[hashIndex];
				newEntries[hashIndex] = currentEntry;
				currentEntry = nextEntry;
			}
		}
		entries = newEntries;
	}
	
	public synchronized void clear() {
		Entry localEntries[] = entries;
		int cap = tableCapacity;
		for(int i=0; i<cap; i++) {
			localEntries[i] = null;
		}
		size = 0;
	}
	
	public native int size();
	
	public boolean isEmpty() {
		return size() == 0;
	}

	public Enumeration keys() {
		return new HashEnumeration(true);
	}

	public Enumeration elements() {
		return new HashEnumeration(false);
	}
	
	public synchronized String toString() {
		StringBuffer buffer = new StringBuffer();
		HashEnumeration entries = new HashEnumeration(false);
		Entry entry;
		while(entries.hasMoreElements()) {
			entry = entries.nextEntry();
			buffer.append("(");
			buffer.append(entry.key);
			buffer.append(',');
			buffer.append(entry.value);
			buffer.append(") ");
		}
		return buffer.toString();
	}
	
	public synchronized boolean contains(Object value) {
		Enumeration values = elements();
		Object val;
		boolean result = false;
		while(values.hasMoreElements()) {
			val = values.nextElement();
			if(val.equals(value)) {
				result = true;
				break;
			}
		}
		return result;
	}
	
	public boolean containsKey(Object key) {
		return get(key) != null;
	}
	
	public synchronized Object get(Object key) {
		int hash = key.hashCode();
		Entry entry = entries[hash % tableCapacity];
		while(entry != null) {
			if(hash == entry.hash && key.equals(entry.key)) {
				return entry.value;
			}
			entry = entry.next;
		}
		return null;
	}
	
	public synchronized Object remove(Object key) {
		int hash = key.hashCode();
		Entry entry = entries[hash % tableCapacity];
		Entry previousEntry = null;
		while(entry != null) {
			if(hash == entry.hash && key.equals(entry.key)) {
				if(previousEntry != null) {
					previousEntry.next = entry.next;
				}
				size--;
				return entry.value;
			}
			previousEntry = entry;
			entry = entry.next;
		}
		return null;
	}
	
	public synchronized Object put(Object key, Object value) {
		if(value == null) {
			throw new NullPointerException();
		}
		int hash = key.hashCode();
		int hashIndex = hash % tableCapacity;
		Entry localEntries[] = entries;
		Entry nextEntry, currentEntry;
		nextEntry = currentEntry = localEntries[hashIndex];
		while(currentEntry != null) {
			if(hash == nextEntry.hash && nextEntry.key.equals(key)) {
				Object previousValue = nextEntry.value;
				nextEntry.value = value;
				return previousValue;
			}
			currentEntry = currentEntry.next;
		}
		
		while(size() + 1 > ((tableCapacity * loadFactor) / 100)) {
			rehash();
			hashIndex = hash % tableCapacity;
			localEntries = entries;
			nextEntry = localEntries[hashIndex];
		}
		
		Entry newEntry = new Entry(key, value, hash, nextEntry);
		localEntries[hashIndex] = newEntry;
		size++;
		return null;
	}
	
	class Entry {
		Entry(Object key, Object value, int hash, Entry next) {
			this.key = key;
			this.value = value;
			this.next = next;
			this.hash = hash;
		}
		int hash;
		Entry next;
		Object key;
		Object value;
	}
	
	private class HashEnumeration implements Enumeration {
		Entry currentElement;
		int currentIndex;
		boolean isKeyEnumeration;
		
		HashEnumeration(boolean isKeyEnumeration) {
			this.isKeyEnumeration = isKeyEnumeration;
			findNextElement();
		}
		
		public boolean hasMoreElements() {
			return currentElement != null;
		}
		
		void findNextElement() {
			Entry nextElement = currentElement;
			if(nextElement != null) {
				nextElement = nextElement.next;
			}
			if(nextElement == null) {
				Entry localEntries[] = entries;
				currentIndex++;
				while(currentIndex < tableCapacity) {
					nextElement = localEntries[currentIndex];
					if(nextElement != null) {
						break;
					}
					currentIndex++;
				}
			}
			currentElement = nextElement;
		}
		
		Entry nextEntry() {
			Entry cElement = currentElement;
			if(cElement == null) {
				throw new NoSuchElementException();
			}
			findNextElement();
			return cElement;
		}
		
 		public Object nextElement() {
			return isKeyEnumeration ? nextEntry().key : nextEntry().value;
		}
	}
}

