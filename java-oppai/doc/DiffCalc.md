# Documentation

## `public class DiffCalc`

The DiffCalc class is used for all calculations related to the difficulty of a beatmap.

 * **Author:** dpisdaniel

## `public static final int DEFAULT_SINGLETAP_THRESHOLD = 125`

## `public void diffCalc(Beatmap b, OppaiCtx diffCtx, boolean withAwkwardness, boolean withAimSingles, boolean withTimingSingles, boolean withThresholdSingles, int singletapThreshold )`

Calculates difficulty (star rating) for a beatmap.

 * **Parameters:**
   * `b` — the Beatmap to calculate the difficulty of.
   * `diffCtx` — oppai context object.
   * `withAwkwardness` — if True, rhythm awkwardness will be calculated, otherwise, the rhythmAwkwardness property will not be set.
   * `withAimSingles` — if True, the number of aim singletaps will be calculated. Aim singletaps are seen by the difficulty calculator (based on 

     a spacing threshold). if False, the nSingles property will not be set.
   * `withTimingSingles` — if True, the number of 1/2 notes will be calculated. Otherwise, the nSinglesTiming property will not be set.
   * `withThresholdSingles` — if True, the number of notes that are 1/2 or slower at singletapThreshold ms will be calculated. Otherwise,

     the nSinglesThreshold property will not be set.
   * `singletapThreshold` — The singletap threshold in ms. Use the default one if you don't care or know what this is. Alternatively,
 * **See also:** Beatmap#diffCalc(OppaiCtx, boolean, boolean, boolean, boolean)

## `public void diffCalc(Beatmap b, OppaiCtx diffCtx, boolean withAwkwardness, boolean withAimSingles, boolean withTimingSingles, boolean withThresholdSingles)`

Calculates difficulty (star rating) for a beatmap.

 * **Parameters:**
   * `b` — the Beatmap to calculate the difficulty of.
   * `diffCtx` — oppai context object.
   * `withAwkwardness` — if True, rhythm awkwardness will be calculated, otherwise, the rhythmAwkwardness property will not be set.
   * `withAimSingles` — if True, the number of aim singletaps will be calculated. Aim singletaps are seen by the difficulty calculator (based on 

     a spacing threshold). if False, the nSingles property will not be set.
   * `withTimingSingles` — if True, the number of 1/2 notes will be calculated. Otherwise, the nSinglesTiming property will not be set.
   * `withThresholdSingles` — if True, the number of notes that are 1/2 or slower at singletapThreshold ms will be calculated. Otherwise,

     the nSinglesThreshold property will not be set.

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