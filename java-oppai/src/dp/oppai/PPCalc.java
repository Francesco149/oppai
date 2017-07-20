package dp.oppai;

/**
 * The PPCalc class is used for all calculations related to pp calculations of a beatmap.
 * 
 * @author dpisdaniel
 *
 */
public class PPCalc {
	
	public static final short DEFAULT_COMBO = (short)0xFFFF;
	public static final short DEFAULT_MISSES = 0;
	public static final short DEFAULT_C300 = (short)0xFFFF;
	public static final short DEFAULT_C100 = 0;
	public static final short DEFAULT_C50 = 0;
	public static final int DEFAULT_SCORE_VER = 1;
	public static final double DEFAULT_ACCURACY = 100.0;
	
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
	
	// A clusterfuck due to java not allowing optional parameters smh
	// TODO: Try to make this work with varargs
	
	/**
	 * Calculates pp for a beatmap.
	 * Sets the results in {@link #accPercent}, {@link #pp}, {@link #aimPP}, {@link #speedPP} and {@link #accPP}
	 * Use other overloads of this function if you don't want to pass some of these values.
	 * @param b the Beatmap to calc the pp for
	 * @param ctx the oppai context object
	 * @param aim aim stars. {@link DiffCalc#aim}
	 * @param speed speed stars. {@link DiffCalc#stars}
	 * @param usedMods any combination of the mod constants bit-wise OR-ed together (example: Mods.HD | Mods.HR) defaults to {@link Mods#NOMOD}
	 * @param combo the combo. defaults to {@link #DEFAULT_COMBO}
	 * @param misses the num. of misses. defaults to {@link #DEFAULT_MISSES}
	 * @param c300 the num. of c300. defaults to {@link #DEFAULT_C300}
	 * @param c100 the num. of c100. defaults to {@link #DEFAULT_C100}
	 * @param c50 the num. of c50. defaults to {@link #DEFAULT_C50}
	 * @param scoreVersion 1 or 2. the default is 1. scorev2 affects accuracy pp
	 */
	public void calcPP(Beatmap b, OppaiCtx ctx, double aim, double speed, int usedMods, int combo, int misses, int c300, int c100, int c50, int scoreVersion) {
		nativeCalcPP(b, ctx, aim, speed, usedMods, combo, misses, c300, c100, c50, scoreVersion);
	}
	
	/**
	 * Overloaded function. 
	 * For the default values see {link #calcPP(Beatmap, OppaiCtx, double, double, int, int, int, int, int, int, int)}
	 */
	public void calcPP(Beatmap b, OppaiCtx ctx, double aim, double speed, int usedMods) {
		nativeCalcPP(b, ctx, aim, speed, usedMods, DEFAULT_COMBO, DEFAULT_MISSES, DEFAULT_C300, DEFAULT_C100, DEFAULT_C50, DEFAULT_SCORE_VER);
	}
	
	/**
	 * Overloaded function. 
	 * For the default values see {link #calcPP(Beatmap, OppaiCtx, double, double, int, int, int, int, int, int, int)}
	 */
	public void calcPP(Beatmap b, OppaiCtx ctx, int aim, int speed, int usedMods, int combo, int misses, int c300, int c100, int c50, int scoreVersion) {
		nativeCalcPP(b, ctx, aim, speed, usedMods, combo, misses, c300, c100, c50, scoreVersion);
	}
	
	/**
	 * Overloaded function. 
	 * For the default values see {link #calcPP(Beatmap, OppaiCtx, double, double, int, int, int, int, int, int, int)}
	 */
	public void calcPP(Beatmap b, OppaiCtx ctx, double aim, double speed) {
		nativeCalcPP(b, ctx, aim, speed, Mods.NOMOD, DEFAULT_COMBO, DEFAULT_MISSES, DEFAULT_C300, DEFAULT_C100, DEFAULT_C50, DEFAULT_SCORE_VER);
	}
	
	/**
	 * Overloaded function.
	 * For more options and an explanation see {@link #calcPPAcc(Beatmap, OppaiCtx, double, double, double, int, int, int, int)}
	 * @param b the Beatmap to calc the pp for
	 * @param ctx oppai ctx object see {@link OppaiCtx#OppaiCtx()}
	 * @param aim aim stars see {@link DiffCalc#diffCalc(Beatmap, OppaiCtx, boolean, boolean, boolean, boolean)}
	 * @param speed speed stars see {@link DiffCalc#diffCalc(Beatmap, OppaiCtx, boolean, boolean, boolean, boolean)}
	 * @param acc the accuracy in percentage to calculate
	 */
	public void calcPPAcc(Beatmap b, OppaiCtx ctx, double aim, double speed, double acc) {
		nativeCalcPPAcc(b, ctx, aim, speed, acc, Mods.NOMOD, DEFAULT_COMBO, DEFAULT_MISSES, DEFAULT_SCORE_VER);
	}
	
	/**
	 * Sets the results in {@link #accPercent}, {@link #pp}, {@link #aimPP}, {@link #speedPP} and {@link #accPP}
	 * Same as {@link #calcPP(Beatmap, OppaiCtx, double, double, int, int, int, int, int, int, int)} but with percentage accuracy.
	 * 
	 * @param b the Beatmap to calc the pp for
	 * @param ctx oppai ctx object see {@link OppaiCtx#OppaiCtx()}
	 * @param aim aim stars see {@link DiffCalc#diffCalc(Beatmap, OppaiCtx, boolean, boolean, boolean, boolean)}
	 * @param speed speed speed stars see {@link DiffCalc#diffCalc(Beatmap, OppaiCtx, boolean, boolean, boolean, boolean)}
	 * @param acc acc the accuracy in percentage to calculate
	 * @param usedMods any combination of the mod constants bit-wise OR-ed together (e.g Mods.HD | Mods.HR) defaults to {@link Mods#NOMOD}
	 * @param combo the combo. defaults to {@link #DEFAULT_COMBO}
	 * @param misses the num. of misses. defaults to {@link #DEFAULT_MISSES}
	 * @param scoreVersion 1 or 2. the default is 1. scorev2 affects accuracy pp
	 */
	public void calcPPAcc(Beatmap b, OppaiCtx ctx, double aim, double speed, double acc, int usedMods, int combo, int misses, int scoreVersion) {
		nativeCalcPPAcc(b, ctx, aim, speed, acc, usedMods, combo, misses, scoreVersion);
	}
	
	private native void nativeCalcPP(Beatmap b, OppaiCtx ctx, double aim, double speed, int usedMods, int combo, int misses, int c300, int c100, int c50, int scoreVersion);
	private native void nativeCalcPPAcc(Beatmap b, OppaiCtx ctx, double aim, double speed, double acc, int usedMods, int combo, int misses, int scoreVersion);
	
	static {
        System.loadLibrary("oppai");
    }
}
