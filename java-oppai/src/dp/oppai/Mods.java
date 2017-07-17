package dp.oppai;
/**
 * Mods contains all the different mods constants for oppai's mods bitmask.
 * 
 * @author dpisdaniel
 * @version 1.0
 */
public final class Mods {
	
	public static final int NOMOD = 0;
	public static final int NF = 1 << 0;
	public static final int EZ = 1 << 1;
	public static final int HD = 1 << 3;
	public static final int HR = 1 << 4;
	public static final int DT = 1 << 6;
	public static final int HT = 1 << 8;
	public static final int NC = 1 << 9;
	public static final int FL = 1 << 10;
	public static final int SO = 1 << 12;
	
	private Mods() {
		//this prevents even the native class from 
	    //calling this ctor as well
		throw new AssertionError();
	}
}
