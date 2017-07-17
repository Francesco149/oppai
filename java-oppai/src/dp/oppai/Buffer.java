package dp.oppai;
/**
 * Buffer represents a buffer used for containing all the beatmap data.
 * 
 * @author dpisdaniel
 * @version 1.0
 */
public class Buffer {
	private int bufferLength;
	private long nativeHandle; // This would be the pointer to the buffer
	
	/**
	 * Constructor for the Buffer class.
	 * Constructs a new buffer to hold the beatmap contents.
	 * 
	 * @param bufferLength the size in bytes of the buffer.
	 */
	public Buffer(int bufferLength) {
		this.bufferLength = bufferLength;
		newBuffer(bufferLength);
	}
	
	/**
	 * Retrieves the buffer's length.
	 * 
	 * @return the size (length) of the buffer in bytes.
	 */
	public int getBufferLength() {
		return this.bufferLength;
	}
	
	/**
	 * Disposes of this buffer instance. 
	 * Use this when this buffer instance is no longer needed.
	 * Sadly it's hard to get rid of our native objects with Java's garbage collector and no ideal alternative is given in java (like C++'s destructors)
	 * so this method is needed. It's very recommended to call this if you don't want memory leaks.
	 * the Buffer instance itself will still exist in the scope it was made, only the native object will no longer exist, so make sure you don't pass this instance 
	 * to any function anymore because the underlying native object will not exist and this will crash your program.
	 */
	public native void dispose();
	
	private native void newBuffer(int bufferLength);
	
	static {
		System.loadLibrary("oppai");
	}
}
