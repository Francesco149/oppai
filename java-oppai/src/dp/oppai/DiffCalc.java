package dp.oppai;

/**
 * The DiffCalc class is used for all calculations related to the difficulty of a beatmap.
 * 
 * @author dpisdaniel
 *
 */
public class DiffCalc {
	
	public static final int DEFAULT_SINGLETAP_THRESHOLD = 125;
	
	private double stars;
	private double aim;
	private double speed;
	private double rhythmAwkwardness;
	private double nSingles;
	private double nSinglesTiming;
	private double nSinglesThreshold;	
	
	/**
	 * Calculates difficulty (star rating) for a beatmap.
	 * @param b the Beatmap to calculate the difficulty of.
	 * @param diffCtx oppai context object.
	 * @param withAwkwardness if True, rhythm awkwardness will be calculated, otherwise, the {@link #rhythmAwkwardness} property will not be set.
	 * @param withAimSingles if True, the number of aim singletaps will be calculated. Aim singletaps are seen by the difficulty calculator (based on 
	 * a spacing threshold). if False, the {@link #nSingles} property will not be set.
	 * @param withTimingSingles if True, the number of 1/2 notes will be calculated. Otherwise, the {@link #nSinglesTiming} property will not be set.
	 * @param withThresholdSingles if True, the number of notes that are 1/2 or slower at singletapThreshold ms will be calculated. Otherwise,
	 * the {@link #nSinglesThreshold} property will not be set.
	 * @param singletapThreshold The singletap threshold in ms. Use the default one if you don't care or know what this is. Alternatively, 
	 * @see Beatmap#diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)
	 */
	public void diffCalc(Beatmap b, OppaiCtx diffCtx, boolean withAwkwardness, boolean withAimSingles, boolean withTimingSingles, boolean withThresholdSingles, int singletapThreshold ) {
		nativeDiffCalc(b, diffCtx, withAwkwardness, withAimSingles, withTimingSingles, withThresholdSingles, singletapThreshold);
	}
	
	/**
	 * Calculates difficulty (star rating) for a beatmap.
	 * @param b the Beatmap to calculate the difficulty of.
	 * @param diffCtx oppai context object.
	 * @param withAwkwardness if True, rhythm awkwardness will be calculated, otherwise, the {@link #rhythmAwkwardness} property will not be set.
	 * @param withAimSingles if True, the number of aim singletaps will be calculated. Aim singletaps are seen by the difficulty calculator (based on 
	 * a spacing threshold). if False, the {@link #nSingles} property will not be set.
	 * @param withTimingSingles if True, the number of 1/2 notes will be calculated. Otherwise, the {@link #nSinglesTiming} property will not be set.
	 * @param withThresholdSingles if True, the number of notes that are 1/2 or slower at singletapThreshold ms will be calculated. Otherwise,
	 * the {@link #nSinglesThreshold} property will not be set.
	 */
	public void diffCalc(Beatmap b, OppaiCtx diffCtx, boolean withAwkwardness, boolean withAimSingles, boolean withTimingSingles, boolean withThresholdSingles) {
		nativeDiffCalc(b, diffCtx, withAwkwardness, withAimSingles, withTimingSingles, withThresholdSingles, DEFAULT_SINGLETAP_THRESHOLD);
	}
	
	private native void nativeDiffCalc(Beatmap b, OppaiCtx diffCtx, boolean withAwkwardness, boolean withAimSingles, boolean withTimingSingles, boolean withThresholdSingles, int singletapThreshold);
	
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
	
	static {
        System.loadLibrary("oppai");
    }
}
