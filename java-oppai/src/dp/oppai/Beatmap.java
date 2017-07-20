package dp.oppai;

/**
 * The Beatmap class represents a beatmap struct from oppai.
 * 
 * @author dpisdaniel
 *
 */
public class Beatmap {
	
	private long nativeHandle; // beatmap handle for internal usage
	
	/**
	 * Constructor for class Beatmap.
	 * Constructs a new beatmap with the given context. 
	 * @param ctx the context to associate the beatmap with.
	 */
	public Beatmap(OppaiCtx ctx) {
		newBeatmap(ctx);
	}
	
	/**
	 * Parses a .osu file into a beatmap object.
	 * @param path the path to the .osu file to parse or "-" to read from stdin
	 * @param buffer the buffer object to use for containing the beatmap data.
	 * @param disableCache passing true will disable caching. Passing false will use caching. 
	 * If false is given, cachePath should also be a valid path.
	 * @param cachePath the path to cache beatmaps at. If no valid path is given, it will use the path of the current exectuable.
	 */
	public void parse(String path, Buffer buffer, boolean disableCache, String cachePath) {
		nativeParse(path, buffer, buffer.getBufferLength(), disableCache, cachePath);
	}
	
	/**
	 * 
	 * @return the CS of the beatmap.
	 */
	public native float getCS();
	/**
	 * 
	 * @return the OD of the beatmap.
	 */
	public native float getOD();
	/**
	 * 
	 * @return the AR of the beatmap.
	 */
	public native float getAR();
	/**
	 * 
	 * @return the HP of the beatmap.
	 */
	public native float getHP();
	
	/**
	 * Sets the CS of the beatmap to the given CS.
	 * @param cs the CS to set the beatmap's CS to.
	 */
	public native void setCS(float cs);
	/**
	 * Sets the OD of the beatmap to the given OD.
	 * @param od the OD to set the beatmap's OD to.
	 */
	public native void setOD(float od);
	/**
	 * Sets the AR of the beatmap to the given AR.
	 * @param ar the AR to set the beatmap's AR to.
	 */
	public native void setAR(float ar);
	
	/**
	 * 
	 * @return the artist of the beatmap.
	 */
	public native String getArtist();
	/**
	 * 
	 * @return the title of the beatmap.
	 */
	public native String getTitle();
	/**
	 * 
	 * @return the version of the beatmap.
	 */
	public native String getVersion();
	/**
	 * 
	 * @return the creator of the beatmap.
	 */
	public native String getCreator();
	
	/**
	 * 
	 * @return the number of objects in the beatmap.
	 */
	public native int getNumObjects();
	/**
	 * 
	 * @return the number of circles in the beatmap.
	 */
	public native int getNumCircles();
	/**
	 * 
	 * @return the number of sliders in the beatmap.
	 */
	public native int getNumSliders();
	/**
	 * 
	 * @return the number of spinners in the beatmap.
	 */
	public native int getNumSpinners();
	/**
	 * 
	 * @return the max combo of the beatmap.
	 */
	public native int getMaxCombo();
	
	/**
	 * 
	 * @return the mode code of the beatmap.
	 */
	public native short getMode();
	
	/**
	 * Applies map-changing mods to a beatmap.
	 * 
	 * @param modMask any combination of the mod constants bit-wise OR-ed together. (e.g Mods.HD | Mods.HR)
	 */
	public native void applyMods(int modMask);
	
	/**
	 * Disposes of this beatmap instance. 
	 * Use this when this beatmap instance is no longer needed.
	 * Sadly it's hard to get rid of our native objects with Java's garbage collector and no ideal alternative is given in java (like C++'s destructors)
	 * so this method is needed. It's very recommended to call this if you don't want memory leaks.
	 * the Beatmap instance itself will still exist in the scope it was made, only the native object will no longer exist, so make sure you don't pass this instance 
	 * to any function anymore because the underlying native object will not exist and this will crash your program.
	 */
	public native void dispose();
	
	private native void newBeatmap(OppaiCtx ctx); // used to initialize the beatmap handle
	private native void nativeParse(String path, Buffer buffer, int bufferLength, boolean disableCache, String cachePath);
	
	/**
	 * Creates a string containing some of the metadata of the beatmap in a formatted way like so:
	 * False Noise - Skyshards[Shroud](by Shiirn)
	 * CS4.0 OD9.0 AR9.4 HP7.4
	 * 1168 objects (601 circles, 565 sliders, 2 spinners)
	 * max combo: 1978
	 * @return a formatted string of the metadata of the beatmap.
	 */
	public String toString() {
		String beatmapInfo = "";
		beatmapInfo += getArtist() + " - " + getTitle() + "[" + getVersion() + "]" + "(by " + getCreator() + ")\n";
		
		beatmapInfo += "CS" + getCS() + " OD" + getOD() + " AR" + getAR() + " HP" +getHP() + "\n";
		beatmapInfo += getNumObjects() + " objects (" + getNumCircles() + " circles, " + getNumSliders() + " sliders, " + getNumSpinners() + " spinners)\n";
		beatmapInfo += "max combo: " + getMaxCombo();
		
		return beatmapInfo;
	}
	
	static {
        System.loadLibrary("oppai");
    }
}
