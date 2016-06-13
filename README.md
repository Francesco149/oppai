osu! pp advanced inspector (oppai) is a difficulty and pp calculator for osu! 
standard beatmaps. It works on any map, even unsubmitted ones and all you have
to do is supply it with the map's .osu file.

# Getting started
Demonstration usage video on windows and linux: 
[here](https://my.mixtape.moe/wasune.webm).

* Download the latest binaries for your OS from 
[here](https://github.com/Francesco149/oppai/releases).
* Extract the archive and place the executable anywhere you like (a good place
would be your osu! folder). Advanced users are free to add oppai to their PATH
to use it anywhere.
* Open cmd (or your favorite terminal emulator if you're on linux) and 
`cd /path/to/your/oppai/folder` (forward slashes might be backwards on 
windows)
* Type `./oppai` for a list of possible parameters.
* Remember that you can tab-complete folder names and file names by pressing 
  tab after typing the first few letters.

# Compiling from source (Linux)
```bash
git clone https://github.com/Francesco149/oppai.git
cd oppai
make
```

To cross compile, you can edit the makefile and add, for example, ```-m32``` 
after g++ in CXX.

# Compiling from source (Windows)
You need to have git bash installed. The easiest way to get it is to install 
GitHub desktop.

You will also need visual studio 2015.

Open git bash and type:

```bash
git clone https://github.com/Francesco149/oppai.git
```

Now navigate to the oppai folder and go to win/oppai. You will fin a visual 
studio 2015 solution file that you can open and compile for x86 and x64.

I recommend compiling in Release mode unless you want all that debug info.

# Compiling from source (MinGW)
You can also use mingw, all you need is git, make and mingw (I personally installed 
them through chocolatey) and /path/to/mingw/bin to your PATH environment
variable.

Open Powershell and type:
```bash
git clone https://github.com/Francesco149/oppai.git
cd oppai/win
make
```

The executable will be in the win directory.
