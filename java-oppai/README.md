Java bindings for oppai. Tested on windows and linux.

# What is this and why should I use it
Bindings are much cleaner and more high-performant than spawning an oppai process,
especially if you need to run calculations on thousands of maps.
It also makes it easier to handle errors and customize behaviour beyond what
the command line tool can do.

# How to install
Make sure you have git to clone this repository. On linux it's usually available
in your package manager if not already installed, while on windows you will need
to install git bash.

## Cloning and compiling the jar

To create the jar file for the library, firstly, make sure you have the jdk tools so you can use commands
like javac, jar and java in your terminal. (you might have to add their location to your PATH)
open up your favorite terminal and do the following:

```bash
git clone https://github.com/Francesco149/oppai.git
cd oppai/java-oppai/src
javac dp/oppai/*.java
jar cf oppai.jar dp/oppai/*.class
```
This will create the jar file that you will refer to in your own project, containing all of the
needed classes.

## Compiling the shared library

Since these are bindings for code that was written in C++, JNI had to be used
to talk to native code. JNI relies on shared libraries, so we'll have to make one.

### For Windows

You will need the MSVC build tools for this step, if you don't have them,
dl them [here](http://landinghub.visualstudio.com/visual-cpp-build-tools)
(oppai was not made to be built on mingw)
Now comes the part where you need to check whether the java version you are using is 
32 or 64 bits, since you will have to compile the DLL accordingly.

You will have to target the compiler's platform to be the same as your java's platform (i.e 32 bit or 64 bit)
Open up a developer command prompt (if you don't have this, download visual studio to install it)
and follow the guide [here](https://msdn.microsoft.com/en-us/library/f2ccy3wt.aspx) to set the compiler's
target platform to java's target platform. (NOTE: this will only be set for this particular command prompt and ocne you close and reopen it,
what you set will be reset back)
Navigate back (in the same dev command prompt) to /oppai/java-oppai/src and run the following:

```bash
cl /D_CRT_SECURE_NO_WARNINGS=1 /DNOMINMAX=1 /DOPPAI_LIB=1 /O2 /nologo /MD /LD /Gm /GR /EHsc 
/W4 /WX /wd4201 /wd4100 /wd4458 /wd4800 /wd4189 /F8000000 
/I"C:\Program Files\Java\jdk1.8.0_131\include" 
/I"C:\Program Files\Java\jdk1.8.0_131\include\win32" 
/Feoppai.dll Beatmap.cpp Advapi32.lib
```

Note that the paths to \include and \include\win32 will change according to where your jdk
was isntalled and whether or not it's for 64 or 32 bits (i.e Program Files(x86) for 32 and Program Files for 64)
so make sure you change these paths if this is different for you, the rest should work.

### For Linux

It's quite simpler to compile on Linux since you can set your architecture and platform from the compiler flags
and you don't have to dl any tools.

Just run this and make sure your platform matches the java platform youre using:
```bash
g++
```

Now that you have both the DLL and the jar file in your directory, you can add them to your project 
by adding the jar to your classpath and the DLL to your java build path 