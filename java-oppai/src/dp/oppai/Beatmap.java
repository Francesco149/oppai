package dp.oppai;

public class Beatmap {
	
	public static final int DEFAULT_SINGLETAP_THRESHOLD = 125;
	public static final short DEFAULT_COMBO = (short)0xFFFF;
	public static final short DEFAULT_MISSES = 0;
	public static final short DEFAULT_C300 = (short)0xFFFF;
	public static final short DEFAULT_C100 = 0;
	public static final short DEFAULT_C50 = 0;
	public static final int DEFAULT_SCORE_VER = 1;
	public static final double DEFAULT_ACCURACY = 100.0;
	
	private long nativeHandle; // beatmap handle for internal usage
	private double stars;
	private double aim;
	private double speed;
	private double rhythmAwkwardness;
	private double nSingles;
	private double nSinglesTiming;
	private double nSinglesThreshold;	
	
	/**
	 * Constructor for class Beatmap.
	 * Constructs a new beatmap with the given context. 
	 * @param ctx the context to associate the beatmap with.
	 */
	public Beatmap(OppaiCtx ctx) {
		newBeatmap(ctx);
	}
	
	/**
	 *
	 * @return the star rating of the beatmap.
	 */
	public double getStars() {
		return stars;
	}
	
	/**
	 *
	 * @return the aim rating of the beatmap.
	 */
	public double getAim() {
		return aim;
	}
	
	/**
	 *
	 * @return the speed rating of the beatmap.
	 */
	public double getSpeed() {
		return speed;
	}
	
	/**
	 * 
	 * @return the rhythm awkwardness of the beatmap.
	 */
	public double getRhythmAwkwardness() {
		return rhythmAwkwardness;
	}
	
	/**
	 * 
	 * @return the number of singles of the beatmap.
	 */
	public double getnSingles() {
		return nSingles;
	}

	/**
	 * 
	 * @return the singles timing of the beatmap.
	 */
	public double getnSinglesTiming() {
		return nSinglesTiming;
	}
	
	/**
	 * 
	 * @return the singles threshold of the beatmap.
	 */
	public double getnSinglesThreshold() {
		return nSinglesThreshold;
	}

	private double accPercent;
	private double pp;
	private double aimPP;
	private double speedPP;
	private double accPP;
	
	/**
	 * 
	 * @return the acc percent of the beatmap.
	 */
	public double getAccPercent() {
		return accPercent;
	}
	
	/**
	 * 
	 * @return the pp of the beatmap given by the acc percent and applied mods.
	 */
	public double getPp() {
		return pp;
	}
	
	/**
	 * 
	 * @return the aim pp of the beatmap given by the acc percent and applied mods.
	 */
	public double getAimPP() {
		return aimPP;
	}
	
	/**
	 * 
	 * @return the speed pp of the beatmap given by the acc percent and applied mods.
	 */
	public double getSpeedPP() {
		return speedPP;
	}
	
	/**
	 * 
	 * @return the acc pp of the beatmap given by the acc percent and applied mods.
	 */
	public double getAccPP() {
		return accPP;
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
	 * Calculates difficulty (star rating) for a beatmap.
	 * @param diffCtx oppai context object
	 * @param withAwkwardness if True, rhythm awkwardness will be calculated, otherwise, the {@link #rhythmAwkwardness} property will not be set.
	 * @param withAimSingles if True, the number of aim singletaps will be calculated. Aim singletaps are seen by the difficulty calculator (based on 
	 * a spacing threshold). if False, the {@link #nSingles} property will not be set.
	 * @param withTimingSingles if True, the number of 1/2 notes will be calculated. Otherwise, the {@link #nSinglesTiming} property will not be set.
	 * @param withThresholdSingles if True, the number of notes that are 1/2 or slower at singletapThreshold ms will be calculated. Otherwise,
	 * the {@link #nSinglesThreshold} property will not be set.
	 * @param singletapThreshold The singletap threshold in ms. Use the default one if you don't care or know what this is. Alternatively, 
	 * @see Beatmap#diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)
	 */
	public void diffCalc(OppaiCtx diffCtx, boolean withAwkwardness, boolean withAimSingles, boolean withTimingSingles, boolean withThresholdSingles, int singletapThreshold ) {
		nativeDiffCalc(diffCtx, withAwkwardness, withAimSingles, withTimingSingles, withThresholdSingles, singletapThreshold);
	}
	
	/**
	 * Calculates difficulty (star rating) for a beatmap.
	 * @param diffCtx oppai context object
	 * @param withAwkwardness if True, rhythm awkwardness will be calculated, otherwise, the {@link #rhythmAwkwardness} property will not be set.
	 * @param withAimSingles if True, the number of aim singletaps will be calculated. Aim singletaps are seen by the difficulty calculator (based on 
	 * a spacing threshold). if False, the {@link #nSingles} property will not be set.
	 * @param withTimingSingles if True, the number of 1/2 notes will be calculated. Otherwise, the {@link #nSinglesTiming} property will not be set.
	 * @param withThresholdSingles if True, the number of notes that are 1/2 or slower at singletapThreshold ms will be calculated. Otherwise,
	 * the {@link #nSinglesThreshold} property will not be set.
	 */
	public void diffCalc(OppaiCtx diffCtx, boolean withAwkwardness, boolean withAimSingles, boolean withTimingSingles, boolean withThresholdSingles) {
		nativeDiffCalc(diffCtx, withAwkwardness, withAimSingles, withTimingSingles, withThresholdSingles, DEFAULT_SINGLETAP_THRESHOLD);
	}
	
	// A clusterfuck due to java not allowing optional parameters smh
	// TODO: Try to make this work with varargs
	
	/**
	 * Calculates pp for a beatmap.
	 * Sets the results in {@link #accPercent}, {@link #pp}, {@link #aimPP}, {@link #speedPP} and {@link #accPP}
	 * Use other overloads of this function if you don't want to pass some of these values.
	 * @param ctx the oppai context object
	 * @param aim aim stars. Default is the {@link Beatmap#aim} of this instance. {@link #diffCalc(OppaiCtx, boolean, boolean, boolean, boolean, int)}
	 * @param speed speed stars. {@link Beatmap#stars} {@link #diffCalc(OppaiCtx, boolean, boolean, boolean, boolean, int)}
	 * @param usedMods any combination of the mod constants bit-wise OR-ed together (example: Mods.HD | Mods.HR) defaults to {@link Mods#NOMOD}
	 * @param combo the combo. defaults to {@link #DEFAULT_COMBO}
	 * @param misses the num. of misses. defaults to {@link #DEFAULT_MISSES}
	 * @param c300 the num. of c300. defaults to {@link #DEFAULT_C300}
	 * @param c100 the num. of c100. defaults to {@link #DEFAULT_C100}
	 * @param c50 the num. of c50. defaults to {@link #DEFAULT_C50}
	 * @param scoreVersion 1 or 2. the default is 1. scorev2 affects accuracy pp
	 */
	public void calcPP(OppaiCtx ctx, double aim, double speed, int usedMods, int combo, int misses, int c300, int c100, int c50, int scoreVersion) {
		nativeCalcPP(ctx, aim, speed, usedMods, combo, misses, c300, c100, c50, scoreVersion);
	}
	
	/**
	 * Overloaded function. 
	 * For the default values see {@link #calcPP(OppaiCtx, double, double, int, int, int, int, int, int, int)}
	 */
	public void calcPP(OppaiCtx ctx, double aim, double speed, int usedMods) {
		nativeCalcPP(ctx, aim, speed, usedMods, DEFAULT_COMBO, DEFAULT_MISSES, DEFAULT_C300, DEFAULT_C100, DEFAULT_C50, DEFAULT_SCORE_VER);
	}
	
	/**
	 * Overloaded function. 
	 * For the default values see {@link #calcPP(OppaiCtx, double, double, int, int, int, int, int, int, int)}
	 */
	public void calcPP(OppaiCtx ctx, int usedMods, int combo, int misses, int c300, int c100, int c50, int scoreVersion) {
		nativeCalcPP(ctx, this.aim, this.speed, usedMods, combo, misses, c300, c100, c50, scoreVersion);
	}
	
	/**
	 * Overloaded function. 
	 * For the default values see {link #calcPP(OppaiCtx, double, double, int, int, int, int, int, int, int)}
	 */
	public void calcPP(OppaiCtx ctx, double aim, double speed) {
		nativeCalcPP(ctx, aim, speed, Mods.NOMOD, DEFAULT_COMBO, DEFAULT_MISSES, DEFAULT_C300, DEFAULT_C100, DEFAULT_C50, DEFAULT_SCORE_VER);
	}
	
	/**
	 * Overloaded function. 
	 * For the default values see {@link #calcPP(OppaiCtx, double, double, int, int, int, int, int, int, int)}
	 */
	public void calcPP(OppaiCtx ctx) {
		nativeCalcPP(ctx, this.aim, this.speed, Mods.NOMOD, DEFAULT_COMBO, DEFAULT_MISSES, DEFAULT_C300, DEFAULT_C100, DEFAULT_C50, DEFAULT_SCORE_VER);
	}
	
	/**
	 * Overloaded function.
	 * For more options and an explanation see {@link #calcPPAcc(OppaiCtx, double, double, double, int, int, int, int)}
	 * 
	 * @param ctx oppai ctx object see {@link OppaiCtx#OppaiCtx()}
	 * @param aim aim stars see #diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)
	 * @param speed speed stars see {@link #diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)}
	 * @param acc the accuracy in percentage to calculate
	 */
	public void calcPPAcc(OppaiCtx ctx, double aim, double speed, double acc) {
		nativeCalcPPAcc(ctx, aim, speed, acc, Mods.NOMOD, Beatmap.DEFAULT_COMBO, Beatmap.DEFAULT_MISSES, Beatmap.DEFAULT_SCORE_VER);
	}
	
	/**
	 * Sets the results in {@link #accPercent}, {@link #pp}, {@link #aimPP}, {@link #speedPP} and {@link #accPP}
	 * Same as {@link #calcPP(OppaiCtx, double, double, int, int, int, int, int, int, int)} but with percentage accuracy.
	 * 
	 * @param ctx oppai ctx object see {@link OppaiCtx#OppaiCtx()}
	 * @param aim aim stars see #diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)
	 * @param speed speed speed stars see {@link #diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)}
	 * @param acc acc the accuracy in percentage to calculate
	 * @param usedMods any combination of the mod constants bit-wise OR-ed together (e.g Mods.HD | Mods.HR) defaults to {@link Mods#NOMOD}
	 * @param combo the combo. defaults to {@link #DEFAULT_COMBO}
	 * @param misses the num. of misses. defaults to {@link #DEFAULT_MISSES}
	 * @param scoreVersion 1 or 2. the default is 1. scorev2 affects accuracy pp
	 */
	public void calcPPAcc(OppaiCtx ctx, double aim, double speed, double acc, int usedMods, int combo, int misses, int scoreVersion) {
		nativeCalcPPAcc(ctx, aim, speed, acc, usedMods, combo, misses, scoreVersion);
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
	private native void nativeDiffCalc(OppaiCtx diffCtx, boolean withAwkwardness, boolean withAimSingles, boolean withTimingSingles, boolean withThresholdSingles, int singletapThreshold);
	private native void nativeCalcPP(OppaiCtx ctx, double aim, double speed, int usedMods, int combo, int misses, int c300, int c100, int c50, int scoreVersion);
	private native void nativeCalcPPAcc(OppaiCtx ctx, double aim, double speed, double acc, int usedMods, int combo, int misses, int scoreVersion);
	
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
