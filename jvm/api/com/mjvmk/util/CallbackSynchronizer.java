package com.mjvmk.util;

import java.util.Hashtable;

/**
** Handles synchronization of a callback in an object oriented fashion
**
** Use this class as follows:
** Given that a method "objectWithCallback.methodWithCallback" will in turn 
** call a callback method "callbackObject.callbackMethod",
** you may transfer data from the callback to the original method as follows:
** 
** Define the callback method as follows:
** class CallBackObject {
** 	public callBackMethod() {
** 		callbackSynchronizer.passCallbackInfo(ID1, new ReturnObject());
** 	}
** }
** 
** The object ret returned from waitForCallBack below will be the same object 
** as that passed to callbackSynchronizer.passCallbackInfo above 
** if ID1.equals(ID2) is true and the callbackSynchronizer object is the same as
** the one used above:
** 
** objectWithCallback.methodWithCallback();
** (ReturnObject) ret = callbackSynchronizer.waitForCallback(ID2);
** 
** You need only be concerned with the uniqueness of the objects ID1 and ID2 
** to ensure proper synchronization.
** 
**
** @version
** @author 	Sean Foley
** @see
** @see
** @since	JDK 1.2
**/


public class CallbackSynchronizer
{
	Object synchronizingObject = this;
	public Hashtable callbackMap = new Hashtable();

	public Object waitForCallback(Object callbackID) {
		Object callbackInfo;
		synchronized (synchronizingObject) {
			callbackInfo = callbackMap.remove(callbackID);
			while(callbackInfo == null) {
				try {
					synchronizingObject.wait();
				}
				catch (InterruptedException e) {}
				callbackInfo = callbackMap.remove(callbackID);
			}
		}
		return callbackInfo;
	}

	public void passCallbackInfo(Object callbackID, Object callbackInfo) {
		synchronized (synchronizingObject) {
			callbackMap.put(callbackID, callbackInfo);
			synchronizingObject.notifyAll();
		}
	}
}

