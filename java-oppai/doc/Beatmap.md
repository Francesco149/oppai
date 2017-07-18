# Beatmap class Documentation

## `public static final int DEFAULT_SINGLETAP_THRESHOLD = 125`
## `public static final short DEFAULT_COMBO = (short)0xFFFF`
## `public static final short DEFAULT_MISSES = 0`
## `public static final short DEFAULT_C300 = (short)0xFFFF`
## `public static final short DEFAULT_C100 = 0`
## `public static final short DEFAULT_C50 = 0`
## `public static final int DEFAULT_SCORE_VER = 1`
## `public static final double DEFAULT_ACCURACY = 100.0`

## `public Beatmap(OppaiCtx ctx)`

Constructor for class Beatmap. Constructs a new beatmap with the given context.

 * **Parameters:** `ctx` — the context to associate the beatmap with.

## `public double getStars()`

 * **Returns:** the star rating of the beatmap.

## `public double getAim()`

 * **Returns:** the aim rating of the beatmap.

## `public double getSpeed()`

 * **Returns:** the speed rating of the beatmap.

## `public double getRhythmAwkwardness()`

 * **Returns:** the rhythm awkwardness of the beatmap.

## `public double getnSingles()`

 * **Returns:** the number of singles of the beatmap.

## `public double getnSinglesTiming()`

 * **Returns:** the singles timing of the beatmap.

## `public double getnSinglesThreshold()`

 * **Returns:** the singles threshold of the beatmap.

## `public double getAccPercent()`

 * **Returns:** the acc percent of the beatmap.

## `public double getPp()`

 * **Returns:** the pp of the beatmap given by the acc percent and applied mods.

## `public double getAimPP()`

 * **Returns:** the aim pp of the beatmap given by the acc percent and applied mods.

## `public double getSpeedPP()`

 * **Returns:** the speed pp of the beatmap given by the acc percent and applied mods.

## `public double getAccPP()`

 * **Returns:** the acc pp of the beatmap given by the acc percent and applied mods.

## `public void parse(String path, Buffer buffer, boolean disableCache, String cachePath)`

Parses a .osu file into a beatmap object.

 * **Parameters:**
   * `path` — the path to the .osu file to parse or "-" to read from stdin
   * `buffer` — the buffer object to use for containing the beatmap data.
   * `disableCache` — passing true will disable caching. Passing false will use caching. 
     If false is given, cachePath should also be a valid path.
   * `cachePath` — the path to cache beatmaps at. If no valid path is given, it will use the path of the current exectuable.

## `public void diffCalc(OppaiCtx diffCtx, boolean withAwkwardness, boolean withAimSingles, boolean withTimingSingles, boolean withThresholdSingles, int singletapThreshold )`

Calculates difficulty (star rating) for a beatmap.

 * **Parameters:**
   * `diffCtx` — oppai context object
   * `withAwkwardness` — if True, rhythm awkwardness will be calculated, otherwise, the rhythmAwkwardness property will not be set.
   * `withAimSingles` — if True, the number of aim singletaps will be calculated. Aim singletaps are seen by the difficulty calculator (based on 

     a spacing threshold). if False, the nSingles property will not be set.
   * `withTimingSingles` — if True, the number of 1/2 notes will be calculated. Otherwise, the nSinglesTiming property will not be set.
   * `withThresholdSingles` — if True, the number of notes that are 1/2 or slower at singletapThreshold ms will be calculated. Otherwise,
     the nSinglesThreshold property will not be set.
   * `singletapThreshold` — The singletap threshold in ms. Use the default one if you don't care or know what this is. Alternatively,
 * **See also:** Beatmap#diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)

## `public void diffCalc(OppaiCtx diffCtx, boolean withAwkwardness, boolean withAimSingles, boolean withTimingSingles, boolean withThresholdSingles)`

Calculates difficulty (star rating) for a beatmap.

 * **Parameters:**
   * `diffCtx` — oppai context object
   * `withAwkwardness` — if True, rhythm awkwardness will be calculated, otherwise, the rhythmAwkwardness property will not be set.
   * `withAimSingles` — if True, the number of aim singletaps will be calculated. Aim singletaps are seen by the difficulty calculator (based on 

     a spacing threshold). if False, the nSingles property will not be set.
   * `withTimingSingles` — if True, the number of 1/2 notes will be calculated. Otherwise, the nSinglesTiming property will not be set.
   * `withThresholdSingles` — if True, the number of notes that are 1/2 or slower at singletapThreshold ms will be calculated. Otherwise,

     the nSinglesThreshold property will not be set.

## `public void calcPP(OppaiCtx ctx, double aim, double speed, int usedMods, int combo, int misses, int c300, int c100, int c50, int scoreVersion)`

Calculates pp for a beatmap. Sets the results in accPercent, pp, aimPP, speedPP and accPP Use other overloads of this function if you don't want to pass some of these values.

 * **Parameters:**
   * `ctx` — the oppai context object
   * `aim` — aim stars. Default is the Beatmap#aim of this instance.
   * `speed` — speed stars. Default is the Beatmap#stars of this instance.
   * `usedMods` — any combination of the mod constants bit-wise OR-ed together (example: Mods.HD | Mods.HR) defaults to Mods#NOMOD
   * `combo` — the combo. defaults to DEFAULT_COMBO
   * `misses` — the num. of misses. defaults to DEFAULT_MISSES
   * `c300` — the num. of c300. defaults to DEFAULT_C300
   * `c100` — the num. of c100. defaults to DEFAULT_C100
   * `c50` — the num. of c50. defaults to DEFAULT_C50
   * `scoreVersion` — 1 or 2. the default is 1. scorev2 affects accuracy pp

## `public void calcPP(OppaiCtx ctx, double aim, double speed, int usedMods)`

Overloaded function. For the default values see #calcPP(OppaiCtx, double, double, int, int, int, int, int, int, int)

## `public void calcPP(OppaiCtx ctx, int usedMods, int combo, int misses, int c300, int c100, int c50, int scoreVersion)`

Overloaded function. For the default values see #calcPP(OppaiCtx, double, double, int, int, int, int, int, int, int)

## `public void calcPP(OppaiCtx ctx, double aim, double speed)`

Overloaded function. For the default values see #calcPP(OppaiCtx, double, double, int, int, int, int, int, int, int)

## `public void calcPP(OppaiCtx ctx)`

Overloaded function. For the default values see #calcPP(OppaiCtx, double, double, int, int, int, int, int, int, int)

## `public void calcPPAcc(OppaiCtx ctx, double aim, double speed, double acc)`

Overloaded function. For more options and an explanation see #calcPPAcc(OppaiCtx, double, double, double, int, int, int, int)

 * **Parameters:**
   * `ctx` — oppai ctx object see OppaiCtx#OppaiCtx()
   * `aim` — aim stars see #diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)
   * `speed` — speed stars see #diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)
   * `acc` — the accuracy in percentage to calculate

## `public void calcPPAcc(OppaiCtx ctx, double aim, double speed, double acc, int usedMods, int combo, int misses, int scoreVersion)`

Sets the results in accPercent, pp, aimPP, speedPP and accPP Same as #calcPP(OppaiCtx, double, double, int, int, int, int, int, int, int) but with percentage accuracy.

 * **Parameters:**
   * `ctx` — oppai ctx object see OppaiCtx#OppaiCtx()
   * `aim` — aim stars see #diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)
   * `speed` — speed speed stars see #diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)
   * `acc` — acc the accuracy in percentage to calculate
   * `usedMods` — any combination of the mod constants bit-wise OR-ed together (e.g Mods.HD | Mods.HR) defaults to Mods#NOMOD
   * `combo` — the combo. defaults to DEFAULT_COMBO
   * `misses` — the num. of misses. defaults to DEFAULT_MISSES
   * `scoreVersion` — 1 or 2. the default is 1. scorev2 affects accuracy pp

## `public native float getCS()`

 * **Returns:** the CS of the beatmap.

## `public native float getOD()`

 * **Returns:** the OD of the beatmap.

## `public native float getAR()`

 * **Returns:** the AR of the beatmap.

## `public native float getHP()`

 * **Returns:** the HP of the beatmap.

## `public native void setCS(float cs)`

Sets the CS of the beatmap to the given CS.

 * **Parameters:** `cs` — the CS to set the beatmap's CS to.

## `public native void setOD(float od)`

Sets the OD of the beatmap to the given OD.

 * **Parameters:** `od` — the OD to set the beatmap's OD to.

## `public native void setAR(float ar)`

Sets the AR of the beatmap to the given AR.

 * **Parameters:** `ar` — the AR to set the beatmap's AR to.

## `public native String getArtist()`

 * **Returns:** the artist of the beatmap.

## `public native String getTitle()`

 * **Returns:** the title of the beatmap.

## `public native String getVersion()`

 * **Returns:** the version of the beatmap.

## `public native String getCreator()`

 * **Returns:** the creator of the beatmap.

## `public native int getNumObjects()`

 * **Returns:** the number of objects in the beatmap.

## `public native int getNumCircles()`

 * **Returns:** the number of circles in the beatmap.

## `public native int getNumSliders()`

 * **Returns:** the number of sliders in the beatmap.

## `public native int getNumSpinners()`

 * **Returns:** the number of spinners in the beatmap.

## `public native int getMaxCombo()`

 * **Returns:** the max combo of the beatmap.

## `public native short getMode()`

 * **Returns:** the mode code of the beatmap.

## `public native void applyMods(int modMask)`

Applies map-changing mods to a beatmap.

 * **Parameters:** `modMask` — any combination of the mod constants bit-wise OR-ed together. (e.g Mods.HD | Mods.HR)

## `public native void dispose()`

Disposes of this beatmap instance. Use this when this beatmap instance is no longer needed. Sadly it's hard to get rid of our native objects with Java's garbage collector and no ideal alternative is given in java (like C++'s destructors) so this method is needed. It's very recommended to call this if you don't want memory leaks. the Beatmap instance itself will still exist in the scope it was made, only the native object will no longer exist, so make sure you don't pass this instance to any function anymore because the underlying native object will not exist and this will crash your program.

## `public String toString()`

Creates a string containing some of the metadata of the beatmap in a formatted way like so: False Noise - Skyshards[Shroud](by Shiirn) CS4.0 OD9.0 AR9.4 HP7.4 1168 objects (601 circles, 565 sliders, 2 spinners) max combo: 1978

 * **Returns:** a formatted string of the metadata of the beatmap.