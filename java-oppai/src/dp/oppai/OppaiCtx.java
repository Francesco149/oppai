package dp.oppai;
/**
 * OppaiCtx represents an ctx struct. It can represent either a regular context or a diff context.
 * 
 * @author dpisdaniel
 * @version 1.0
 */
public class OppaiCtx {
	private long nativeHandle;
	
	/**
	 * Constructor for class OppaiCtx.
	 * Constructs a new context to use with a beatmap.
	 */
	public OppaiCtx() { // This will create a regular ctx
		newCtx();
	}
	
	/**
	 * Constructor for class OppaiCtx.
	 * Constructs a new difficulty context to use when calculating beatmap difficulty
	 * 
	 * @param ctx - The context that is associated with 
	 */
	public OppaiCtx(OppaiCtx ctx) { // This will create a diff ctx
		newDiffCalcContext(ctx);
	}
	
	/**
	 * Retrieves the handle for this context.
	 * @return the handle for this context.
	 */
	public long getContextHandle() {
		return nativeHandle;
	}
	
	/**
	 * Retrieves the last error from this context. 
	 * If no error is found, returns an empty string.
	 * @return The last error's string.
	 */
	public String getLastErr() {
		return nativeGetLastErr();
	}
	
	private native void newCtx();
	
	private native void newDiffCalcContext(OppaiCtx ctx);
	
	private native String nativeGetLastErr();
	
	/**
	 * Disposes of this OppaiCtx instance. 
	 * Use this when this OppaiCtx instance is no longer needed.
	 * Sadly it's hard to get rid of our native objects with Java's garbage collector and no ideal alternative is given in java (like C++'s destructors)
	 * so this method is needed. It's very recommended to call this if you don't want memory leaks.
	 * the OppaiCtx instance itself will still exist in the scope it was made, only the native object will no longer exist, so make sure you don't pass this instance 
	 * to any function anymore because the underlying native object will not exist and this will crash your program.
	 */
	public native void dispose();
	
	static {
        System.loadLibrary("oppai");
    }
}
