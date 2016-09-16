package com.mjvmk.util;

/**
** Base class for enumerator classes
** To be used for enumeration data, similar to the enum type in C
** Extend this class to define an enumerator class type
** There will be exactly one static final object to represent each integer in an enumeration
** If you wish, a single object in the enumeration may represent several integers
** One of the main benefits of extending this class is simplicity, you need to do very little coding
** to create a simple or even a fairly complex enumeration
** It is also designed for efficiency and flexibility.
** 
**
**
** @version September 12, 1999
** @author 	Sean Foley
** @see
** @see
** @since	JDK 1.2
**/
abstract public class Enum
{
	int value;
	int alternateValues[];
	String label; //descriptive label for the enum value
	String alternateLabels[]; //alternate labels for the enum value
	
	private EnumGroup enumGroup;

	Enum() {}
	
	protected Enum(int value, EnumGroup enumGroup) {
		this(value, null, null, null, enumGroup);
	}
	
	protected Enum(int value, String label, EnumGroup enumGroup) {
		this(value, null, label, null, enumGroup);
	}
	
	protected Enum(int value, String label, String alternateLabels[], EnumGroup enumGroup) {
		this(value, null, label, alternateLabels, enumGroup);
	}
	
	protected Enum(int value, int alternateValues[], EnumGroup enumGroup) {
		this(value, alternateValues, null, null, enumGroup);
	}
	
	protected Enum(int value, int alternateValues[], String label, EnumGroup enumGroup) {
		this(value, alternateValues, label, null, enumGroup);
	}
	

	/**
	 * Creates an object in the enumeration
	 * The same enumProperties object must be passed to all Enum object constructors of the same subclass type
	 * @param value the integer corresponding to this object in the enumeration
	 * @param alternateValues[] this object may represent more than one integer, specify the others here, may be null
	 * @param label a string that describes this object in the enumeration, the fromString method uses these values, may be null
	 * @param alternateLabels[] alternate labels which describe the object, the fromString method uses these values, may be null
	 * @param enumProperties the object which contain methods corresponding to the entire enumeration
	 * @see com.logica.sms.oam.util.Enum
	 */
	protected Enum(int value, int alternateValues[], String label, String alternateLabels[], EnumGroup enumGroup) {
		setValues(value, alternateValues, label, alternateLabels, enumGroup);
	}
	
	void setValues(int value, int alternateValues[], String label, String alternateLabels[], EnumGroup enumGroup) {
		this.value=value;
		this.label=label;
		this.alternateLabels=alternateLabels;
		this.enumGroup = enumGroup;
		this.alternateValues = alternateValues;
		enumGroup.addEnum(this);
	}

	/**
	 * Returns the EnumGroup object which is common to all Enums of the same subclass type 
	 *
	 * @return the EnumGroup object
	 */
	public EnumGroup getEnumGroup() {
		return enumGroup;
	}
	
	
	/**
	 * Compares this enum with any other
	 * This method is consistent with the compareTo method
	 * @param object 
	 * @return true if object is the same object as this one, false otherwise
	*/
	public boolean equals(Object object) {
		return object == this;
	}
	
	

	/**
	 * Returns the integer that this Enum object represents in the enumeration
	 * @return the integer
	 */
	public int getValue() {
		return value;
	}
	
	/**
	 * Returns an Integer object that represents this Enum object in the enumeration.
	 *
	 * The base functionality of Enum allows one to enumerate integers, representing each integer with an Enum object.
	 * By overriding this method you may allow the enumeration to represent objects of any class type.
	 * 
	 *
	 * For example, if the enumeration represents true or false then you may wish to return java.lang.Boolean objects from this method.
	 * If the Enum class is an enumeration of colors then you may wish to return java.awt.Color objects.
	 * If the Enum class is an enumeration of web sites than you may wish to return a java.net.URL for each site.
	 *
	 * To easiest way to make this possible is to override the methods mapIntToObject() and mapObjectToInt() in a subclass of EnumProperties
	 *
	 * Otherwise, you can override the methods EnumPropeties.isEnumerated(Object), EnumProperties.fromObject(Object) in a subclass of EnumProperties and override this method in your Enumeration class
	 *
	 * 
	 * Override this method if you wish to customize how each object is represented 
	 * @return the Object
	 */
	public Object getObject() {
		return enumGroup.mapIntToObject(value);
	}


	/**
	 * Returns any other integers this Enum object may represent
	 * @return the array of integers (not including that returned by getValue()), or null if this object represents exactly one integer
	 */
	public int[] getAlternateValues() {
		return alternateValues;
	}

	/**
	 * @return the label specified in the constructor or setLabel, returns null of no such label was specified
	 */
	public String getLabel() {
		return label;
	}

	/**
	 * @return the array of alternate labels specified in the constructor or setAlternateLabels, returns null of no such label was specified
	 */
	public String[] getAlternateLabels() {
		return alternateLabels;
	}

	/**
	 * Returns a string representing the object in the enumeration 
	 * @return returns the label specified in the constructor if a non-null label was specified, otherwise the result of Object.toString() is returned
	 */
	public String toString() {
		if(label != null)
			return label;
		return super.toString();
	}
	
	
	/**
	 * Compares one Enum to another based upon the integer value they each represent
	 * @param object Another Enum object that this object will be compared to
	 * @return a negative integer if this obect is less than the specified object, 0 if equal, and a positive integer if greater
	 * @see java.lang.Comparable
	 */
	public int compareTo(Object object) {
		//will throw a ClassCastException if object is not an instance of Enum, as desired (see the description of the Comparable interface)
		return value - ((Enum) object).value;
	}
	
}
	
