# Documentation

## `public class PPCalc`

The PPCalc class is used for all calculations related to pp calculations of a beatmap.

 * **Author:** dpisdaniel
 
## `public static final short DEFAULT_COMBO = (short)0xFFFF`
## `public static final short DEFAULT_MISSES = 0`
## `public static final short DEFAULT_C300 = (short)0xFFFF`
## `public static final short DEFAULT_C100 = 0`
## `public static final short DEFAULT_C50 = 0`
## `public static final int DEFAULT_SCORE_VER = 1`
## `public static final double DEFAULT_ACCURACY = 100.0`

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

## `public void calcPP(Beatmap b, OppaiCtx ctx, double aim, double speed, int usedMods, int combo, int misses, int c300, int c100, int c50, int scoreVersion)`

Calculates pp for a beatmap. Sets the results in accPercent, pp, aimPP, speedPP and accPP Use other overloads of this function if you don't want to pass some of these values.

 * **Parameters:**
   * `b` — the Beatmap to calc the pp for
   * `ctx` — the oppai context object
   * `aim` — aim stars. 
   * `speed` — speed stars.
   * `usedMods` — any combination of the mod constants bit-wise OR-ed together (example: Mods.HD | Mods.HR) defaults to Mods#NOMOD
   * `combo` — the combo. defaults to #DEFAULT_COMBO
   * `misses` — the num. of misses. defaults to #DEFAULT_MISSES
   * `c300` — the num. of c300. defaults to #DEFAULT_C300
   * `c100` — the num. of c100. defaults to #DEFAULT_C100
   * `c50` — the num. of c50. defaults to #DEFAULT_C50
   * `scoreVersion` — 1 or 2. the default is 1. scorev2 affects accuracy pp

## `public void calcPP(Beatmap b, OppaiCtx ctx, double aim, double speed, int usedMods)`

Overloaded function. For the default values see {link #calcPP(Beatmap, OppaiCtx, double, double, int, int, int, int, int, int, int)}

## `public void calcPP(Beatmap b, OppaiCtx ctx, int aim, int speed, int usedMods, int combo, int misses, int c300, int c100, int c50, int scoreVersion)`

Overloaded function. For the default values see {link #calcPP(Beatmap, OppaiCtx, double, double, int, int, int, int, int, int, int)}

## `public void calcPP(Beatmap b, OppaiCtx ctx, double aim, double speed)`

Overloaded function. For the default values see {link #calcPP(Beatmap, OppaiCtx, double, double, int, int, int, int, int, int, int)}

## `public void calcPPAcc(Beatmap b, OppaiCtx ctx, double aim, double speed, double acc)`

Overloaded function. For more options and an explanation see #calcPPAcc(Beatmap, OppaiCtx, double, double, double, int, int, int, int)

 * **Parameters:**
   * `b` — the Beatmap to calc the pp for
   * `ctx` — oppai ctx object see OppaiCtx#OppaiCtx()
   * `aim` — aim stars see DiffCalc#diffCalc(Beatmap, OppaiCtx, boolean, boolean, boolean, boolean)
   * `speed` — speed stars see DiffCalc#diffCalc(Beatmap, OppaiCtx, boolean, boolean, boolean, boolean)
   * `acc` — the accuracy in percentage to calculate

## `public void calcPPAcc(Beatmap b, OppaiCtx ctx, double aim, double speed, double acc, int usedMods, int combo, int misses, int scoreVersion)`

Sets the results in accPercent, pp, aimPP, speedPP and accPP Same as #calcPP(Beatmap, OppaiCtx, double, double, int, int, int, int, int, int, int) but with percentage accuracy.

 * **Parameters:**
   * `b` — the Beatmap to calc the pp for
   * `ctx` — oppai ctx object see OppaiCtx#OppaiCtx()
   * `aim` — aim stars see DiffCalc#diffCalc(Beatmap, OppaiCtx, boolean, boolean, boolean, boolean)
   * `speed` — speed speed stars see DiffCalc#diffCalc(Beatmap, OppaiCtx, boolean, boolean, boolean, boolean)
   * `acc` — acc the accuracy in percentage to calculate
   * `usedMods` — any combination of the mod constants bit-wise OR-ed together (e.g Mods.HD | Mods.HR) defaults to Mods#NOMOD
   * `combo` — the combo. defaults to #DEFAULT_COMBO
   * `misses` — the num. of misses. defaults to #DEFAULT_MISSES
   * `scoreVersion` — 1 or 2. the default is 1. scorev2 affects accuracy pp