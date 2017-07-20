# Documentation

## `public class Buffer`

Buffer represents a buffer used for containing all the beatmap data.

 * **Author:** dpisdaniel

## `public Buffer(int bufferLength)`

Constructor for the Buffer class. Constructs a new buffer to hold the beatmap contents.

 * **Parameters:** `bufferLength` — the size in bytes of the buffer.

## `public int getBufferLength()`

Retrieves the buffer's length.

 * **Returns:** the size (length) of the buffer in bytes.

## `public native void dispose()`

Disposes of this buffer instance. Use this when this buffer instance is no longer needed. Sadly it's hard to get rid of our native objects with Java's garbage collector and no ideal alternative is given in java (like C++'s destructors) so this method is needed. It's very recommended to call this if you don't want memory leaks. the Buffer instance itself will still exist in the scope it was made, only the native object will no longer exist, so make sure you don't pass this instance to any function anymore because the underlying native object will not exist and this will crash your program.