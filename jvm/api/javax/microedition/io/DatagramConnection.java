package javax.microedition.io;

/*
 * Author: Sean Foley
 * Created: April 13, 2002 12:40:09 AM
 * Modified: April 13, 2002 12:40:09 AM
 */


public interface DatagramConnection extends Connection
{
	int getMaximumLength(); 
 	int getNominalLength(); 
 	Datagram newDatagram(byte[] buf, int size);
 	Datagram newDatagram(byte[] buf, int size, String addr); 
 	Datagram newDatagram(int size); 
 	Datagram newDatagram(int size, String addr); 
 	void receive(Datagram dgram);
 	void send(Datagram dgram);
}

