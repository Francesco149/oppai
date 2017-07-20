# Documentation

## `public class Beatmap`

The Beatmap class represents a beatmap struct from oppai.

 * **Author:** dpisdaniel


## `public Beatmap(OppaiCtx ctx)`

Constructor for class Beatmap. Constructs a new beatmap with the given context.

 * **Parameters:** `ctx` — the context to associate the beatmap with.

## `public void parse(String path, Buffer buffer, boolean disableCache, String cachePath)`

Parses a .osu file into a beatmap object.

 * **Parameters:**
   * `path` — the path to the .osu file to parse or "-" to read from stdin
   * `buffer` — the buffer object to use for containing the beatmap data.
   * `disableCache` — passing true will disable caching. Passing false will use caching. 

     If false is given, cachePath should also be a valid path.
   * `cachePath` — the path to cache beatmaps at. If no valid path is given, it will use the path of the current exectuable.

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