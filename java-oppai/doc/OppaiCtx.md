# Documentation

## `public class OppaiCtx`

OppaiCtx represents an ctx struct. It can represent either a regular context or a diff context.

 * **Author:** dpisdaniel
 * **Version:** 1.0

## `public OppaiCtx()`

Constructor for class OppaiCtx. Constructs a new context to use with a beatmap.

## `public OppaiCtx(OppaiCtx ctx)`

Constructor for class OppaiCtx. Constructs a new difficulty context to use when calculating beatmap difficulty

 * **Parameters:** `ctx` — - The context that is associated with

## `public long getContextHandle()`

Retrieves the handle for this context.

 * **Returns:** the handle for this context.

## `public String getLastErr()`

Retrieves the last error from this context. If no error is found, returns an empty string.

 * **Returns:** The last error's string.

## `public native void dispose()`

Disposes of this OppaiCtx instance. Use this when this OppaiCtx instance is no longer needed. Sadly it's hard to get rid of our native objects with Java's garbage collector and no ideal alternative is given in java (like C++'s destructors) so this method is needed. It's very recommended to call this if you don't want memory leaks. the OppaiCtx instance itself will still exist in the scope it was made, only the native object will no longer exist, so make sure you don't pass this instance to any function anymore because the underlying native object will not exist and this will crash your program.