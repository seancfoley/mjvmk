package java.io;

/*
 * Author: Sean Foley
 * Created: March 28, 2002 12:30:24 AM
 * Modified: March 28, 2002 12:30:24 AM
 */


public class PrintStream extends OutputStream
{
	private OutputStream outputStream;
	private OutputStreamWriter writer;
	private boolean errorSet;
	
	public PrintStream(OutputStream out) {
		if(out == null) {
			throw new NullPointerException();
		}
		this.outputStream = out;
		writer = new OutputStreamWriter(out);
	}
	
	public void write(int b) {
		try {
			outputStream.write(b);
		}
		catch(IOException e) {
			errorSet = true;
		}
	}

	public void write(byte[] buf, int off, int len) {
		try {
			outputStream.write(buf, off, len);
		}
		catch(IOException e) {
			errorSet = true;
		}
	}

	protected void setError() {
		errorSet = true;
	}
	
	public boolean checkError() {
		flush();
		return errorSet;
	}
	
	public void close() {
		try {
			flush();
			outputStream.close();
		}
		catch(IOException e) {
			errorSet = true;
		}
	}
	
	public void flush() {
		try {
			outputStream.flush();
		}
		catch(IOException e) {
			errorSet = true;
		}
	}
	
	public void print(boolean b) {
		print(b ? "true" : "false");
	}
	
	public void print(String s) {
		if(s == null) {
			s = "null";
		}
		try {
			writer.write(s);
		}
		catch(IOException e) {
			errorSet = true;
		}
	}
	
	public void print(char c) {
		try {
			outputStream.write(c);
		}
		catch(IOException e) {
			errorSet = true;
		}
	}
	
	public void print(int i) {
		print(String.valueOf(i));
	}
	
	public void print(long l) {
		print(String.valueOf(l));
	}
	
	public void print(char[] s) {
		try {
			writer.write(s, 0, s.length);
		}
		catch(IOException e) {
			errorSet = true;
		}
	}
	
	public void print(Object obj) {
		try {
			writer.write(String.valueOf(obj));
		}
		catch(IOException e) {
			errorSet = true;
		}
	}

	public void println() {
		try {
			outputStream.write('\n');
		}
		catch(IOException e) {
			errorSet = true;
		}
	}

	public void println(boolean x) {
		print(x);
		println();
	}

	public void println(char x) {
		print(x);
		println();
	}

	public void println(int x) {
		print(x);
		println();
	}

	public void println(long x) {
		print(x);
		println();
	}

	public void println(char x[]) {
		print(x);
		println();
	}

	public void println(String x) {
		print(x);
		println();
	}

	public void println(Object x) {
		print(x);
		println();
	}
}

