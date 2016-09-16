package com.mjvmk.util;

/**
** To be used in conjunction with Enum
**
** Defines properties of an Enum class
** Handles methods that are collective to an Enum class
** See Enum class for examples on use
**
** @version
** @author 	Sean Foley
** @see
** @see
** @since	JDK 1.2
**/

import java.util.Hashtable;
import java.util.Vector;

public class EnumGroup
{
	private Class enumSubClass;
	
	private Vector list = new Vector();
	private Hashtable valueMap = new Hashtable();
	private Hashtable labelMap = new Hashtable();
	private Hashtable alternateLabelMap = new Hashtable();
	private boolean dynamic;

	public EnumGroup(Class enumSubClass) {
		this(enumSubClass, false);
	}

	/**
	 * Creates an object which is common to all objects in an enumeration
	 * @param enumSubClass The class object corresponding to your subclass of Enum
	 * @param dynamic if true, the fromInt method will dynamically create new objects in the enumeration, see the description of the fromInt method
	 * 	If dynamic is true, then ensure that the class enumSubClass is public and has a public constructor which takes the arguments (int, EnumProperties)
	 */
	public EnumGroup(Class enumSubClass, boolean dynamic) {
		this.enumSubClass = enumSubClass;
		this.dynamic = dynamic;
	}
	
	void addEnum(Enum enum) {
		if(!enum.getClass().equals(enumSubClass)) {
			throw new IllegalArgumentException("All objects in an enumeration must be instances of the same subclass of Enum");
		}
		int numValues = (enum.alternateValues != null ? enum.alternateValues.length + 1 : 1);
		
		for(int j=0; j < numValues; j++) {
			int value = (j==0 ? enum.value : enum.alternateValues[j-1]);
			Integer key = new Integer(value);
			if(valueMap.get(key) != null) {
				throw new IllegalArgumentException("Attempting to create Enum class with duplicate enumeration value " + enum.value + "; existing enum: " + valueMap.get(key) + "; new enum: " + enum);
			}
			valueMap.put(key, enum);
		}
		
		//add the label to the label maps
		if(enum.label != null) {
			labelMap.put(enum.label, enum);
		}
		
		//add all the alternate labels to the alternate label maps
		if(enum.alternateLabels != null) {
			for(int i=0; i<enum.alternateLabels.length; i++) {
				alternateLabelMap.put(enum.alternateLabels[i], enum);
			}
		}
		
		
		boolean added = false;
		int count = list.size();
		for(int i=0; i<count; i++) {
			if(enum.compareTo(list.elementAt(i)) < 0) {
				list.insertElementAt(enum, i);
				added = true;
				break;
			}
		}
		if(!added) {
			list.addElement(enum);
		}
	}
	
	/* after calling this method, the EnumProperties object will no longer be aware of the existence of the specified enum */
	public void removeEnum(Enum enum) {
		if(!list.contains(enum)) {
			return;
		}
			
		Integer key = new Integer(enum.value);
		valueMap.remove(key);
		
		//remove the labels from the label maps
		if(enum.label != null) {
			labelMap.remove(enum.label);
		}
		
		if(enum.alternateLabels!=null) {
			for(int i=0; i<enum.alternateLabels.length; i++) {
				alternateLabelMap.remove(enum.alternateLabels[i]);
			}
		}
		//remove the enum from the list
		list.removeElement(enum);
	}

	/**
	 * Finds the Enum object corresponding to the integer value.
	 *
	 * If you have created a dynamic enumeration (See constructor), then if an object is not found, this method will automatically create
	 * a new object for you to represent the integer.  The object will be of the same class as all the other objects in the enumeration.
	 * For this to be possible, your enumeration class must be public, and it must have a default constructor.
	 * This dynamic creation of Enum objects is thread-safe.
	 *
	 * If the enumeration is not dynamic and the integer is not represented by an object in the enumeration, null is returned
	 * @param value the integer that you wish to match
	 * @return the corresponding Enum object
	 */
	public Enum fromInt(int value) {
		if(dynamic) {
			try {
				return dynamicFromInt(value);
			}
			catch(InstantiationException e) {}
			catch(IllegalAccessException e) {}
			return null;
		}
		return staticFromInt(value);
	}
	
	public Enum dynamicFromInt(int value) throws InstantiationException, IllegalAccessException {
		synchronized(this) {
			Enum ret = staticFromInt(value);
			if(ret == null) { //generate a new one
				ret = (Enum) enumSubClass.newInstance();
				ret.setValues(value, null, null, null, this);
			}
			return ret;
		}
	}
	
	public Enum staticFromInt(int value) {
		return (Enum) valueMap.get(new Integer(value));
	}

	/**
	 * This method controls the behaviour of Enum.getObject()
	 * The current behaviour is ints are mapped to Integer objects
	 * Override this method and mapObjectToInt if you want your enumeration to represent other objects 
	 * @param value the int to be mapped
	 * @return the mapped object
	 */
	protected Object mapIntToObject(int value) {
		return new Integer(value);
	}
	
	/**
	 * This method controls the behaviour of EnumProperties.isEnumerated(Object) and EnumProperties.fromObject(Object)
	 * The current behaviour is: Objects passed to this method must be java.lang.Integer objects, and these are mapped to ints using Integer.intValue()
	 *
	 * Override this method and mapIntToObject if you want your enumeration to represent other objects 
	 * @param value the Object to be mapped, throws ClassCastException if something other than a java.lang.Integer object
	 * @return the mapped int
	 */
	protected int mapObjectToInt(Object object) {
		return ((Integer) object).intValue();
	}
	

	/**
	 * Behaves same way as fromInt(int) method.
	 * Calls fromInt(mapObjectToInt(object))
	 *
	 * @param object the object for which you wish to obtain an Enum
	 * @return the Enum object representing the indicated object
	 */
	public Enum fromObject(Object object) {
		return fromInt(mapObjectToInt(object));
	}
	
	/**
	 * Behaves same way as isEnumerated(int) method.
	 * Calls isEnumerated(mapObjectToInt(object))
	 *
	 * @param object for which may be represented by an Enum
	 * @return whether the Enum which should represent the object is in the enumeration
	 */
	public boolean isEnumerated(Object object) {
		return isEnumerated(mapObjectToInt(object));
	}

	/**
	 * Finds an object in the enumeration corresponding to the indicated String
	 * Will search through all the labels specified by the objects, if that fails then it will search through the alternate labels
	 * @param value the string that you wish to match
	 * @return the corresponding object in the enumeration, null if no such object exists
	 */
	public Enum fromString(String value) {
		Enum ret;
		synchronized(this) {
			String newValue = value;
			ret = (Enum) labelMap.get(newValue);
			if (ret == null) {
				ret = (Enum) alternateLabelMap.get(newValue);
			}
		}
		return ret;
	}

	/**
	 * @return the number of objects currently in the enumeration
	 */
	public int count() {
		return list.size();
	}
	

	/**
	 * @return an array of all objects currently in the enumeration, sorted by the integer values they represent
	 */
	public Enum[] getAllEnums() {
		//create a new array for all the enums and then populate it 
		Enum staticArray[] = new Enum[list.size()];
		list.copyInto(staticArray);
		return staticArray;
	}

	/**
	 * @param value the integer to be represented
	 * @return true if the integer value is currently represented by an object in the enumeration
	 */
	public boolean isEnumerated(int value) {
		return valueMap.containsKey(new Integer(value));
	}
}

