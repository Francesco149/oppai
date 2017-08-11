[![Build Status](https://travis-ci.org/Francesco149/oppai.svg?branch=master)](https://travis-ci.org/Francesco149/oppai)

osu! pp advanced inspector (oppai) is a difficulty and pp calculator for osu!
standard beatmaps. It works on any map, even unsubmitted ones and all you have
to do is supply it with the map's .osu file.

- [Getting started](#getting-started)
- [Caching beatmaps](#caching-beatmaps)
- [Compiling from source (Linux)](#compiling-from-source-linux)
- [Compiling from source (Windows)](#compiling-from-source-windows)
- [Compiling from source (OSX)](#compiling-from-source-osx)
- [Library mode and bindings](#library-mode-and-bindings)

# Getting started
Demonstration usage video on windows and linux:
[here](https://my.mixtape.moe/wasune.webm).

* If you are on arch linux, you can use the AUR packages
[oppai](https://aur.archlinux.org/packages/oppai/) or
[oppai-git](https://aur.archlinux.org/packages/oppai-git/) maintained by
ammongit.
Otherwise, download the latest binaries for your OS from
[here](https://github.com/Francesco149/oppai/releases), extract the archive
and place the executable anywhere you like. Advanced users are free to add oppai
to their PATH to use it anywhere.
* Open cmd (or your favorite terminal emulator if you're on linux) and
`cd /path/to/your/oppai/folder` (forward slashes might be backwards on
windows)
* Type `./oppai` for a list of possible parameters.

UPDATE:
You can now pipe beatmaps to oppai from stdin, which means that you can download
a map on the fly and call oppai on it in one simple command.

Some linux examples of piping:
```bash
curl https://osu.ppy.sh/osu/37658 | ./oppai -
curl https://osu.ppy.sh/osu/37658 | ./oppai - +HDHR
curl https://osu.ppy.sh/osu/37658 | ./oppai - +HDHR 99% 600x 1m
```

Windows examples of piping (using powershell):
```powershell
(New-Object System.Net.WebClient).DownloadString("https://osu.ppy.sh/osu/37658") | ./oppai -
(New-Object System.Net.WebClient).DownloadString("https://osu.ppy.sh/osu/37658") | ./oppai - +HDHR
(New-Object System.Net.WebClient).DownloadString("https://osu.ppy.sh/osu/37658") | ./oppai - +HDHR 99% 600x 1m
```

NOTE: to obtain the beatmap url, just open the desired map's page in your
browser, click on the desired difficulty and copy the url, then replace /b/ with
/osu/.

If you don't feel like using the command line, you can hop on the
[shigetora chat](https://www.twitch.tv/shigetora) and type
`!oppai url_to_beatmap` followed by the parameters. But remember, you won't get
the full output which contains much more useful info than just pp! So I
recommend spending 1 minute downloading the tool and learning to use it from
the command line.

Examples:
```
!oppai https://osu.ppy.sh/osu/37658
!oppai https://osu.ppy.sh/osu/37658 +HDHR
!oppai https://osu.ppy.sh/osu/37658 +HDHR 500x 1m
```

# Caching beatmaps
oppai automatically caches pre-parsed beatmaps in binary format in the
"oppai\_cache" folder in the same directory as the oppai executable.

This speeds up subsequent calculations for that same map by up to 25%,
especially on slow CPU's. The cache filesize is about 65kb per map on average.

If you have a machine with really slow I/O (networked filesystem, old HDD) or
a cpu with really good single core performance, you might want to try passing
```-no-cache``` to disable caching and see if that makes it faster (it usually
makes it SLOWER though).

NOTE: cache files are not guaranteed to be compatible across architecture, so
      you can't copy your beatmap cache from a 64-bit machine to a 32-bit
      machine or use 32-bit and 64-bit oppai on the same cache folder.
      delete it or move it in such cases.

# Compiling from source (Linux)
```bash
git clone https://github.com/Francesco149/oppai.git
cd oppai
./build.sh
```

To cross compile, you can edit the build.sh and add, for example, ```-m32```
after g++ in CXX.

OpenSSL is required to build oppai on Linux. The library and header files are
bundled in the package `libssl-dev` on most distributions. On Debian it can be
obtained as follows:

```bash
sudo apt-get install libssl-dev
```

# Compiling from source (Windows)
You need to have git bash installed. The easiest way to get it is to install
GitHub desktop.

You will also need visual studio. Any version should do. You don't even need the
IDE, you can even get the stand-alone compiler without all the bloat.

Open git bash and type:

```bash
git clone https://github.com/Francesco149/oppai.git
```

Now open a visual studio command prompt:
```bash
cd \path\to\oppai\win
build.bat
```

The executable will be found in the build directory.

# Compiling from source (OSX)
Via homebrew:
```bash
brew install --HEAD pmrowla/homebrew-tap/oppai
```
Note that installing with ```--HEAD``` is not required, but it is recommended
since the tap may not always be up to date with the current oppai release
tarball. Installing from homebrew will place the executable in your homebrew
path.

Compiling from source in OSX will still require the use of homebrew since Apple
no longer bundles OpenSSL headers with OSX. Also note that homebrew may give
warnings about not symlinking OpenSSL, this is intended behavior and you should
not ```brew link --force openssl``` due to potential conflicts with the Apple
bundled OpenSSL.
```bash
brew install openssl
git clone https://github.com/Francesco149/oppai.git
cd oppai
./build.sh
```

# Library mode and bindings
oppai can now be compiled in library mode by defining OPPAI_LIB=1. this allows
you to ```#include /path/to/oppai/code/main.cc``` in any C++ program and call
oppai functions to parse beatmaps, calculate pp and difficulty in your own
software. The API is thread safe, so you can process beatmaps in parallel if
you wish so.

This also makes it simple to create bindings to use oppai in pretty much any
programming language.

Currently available bindings:
* pyoppai (Python): https://github.com/Francesco149/oppai/tree/master/pyoppai
* node-oppai (node.js): https://github.com/Francesco149/oppai/tree/master/node-oppai

The library mode API is still new and nothing is set in stone, but I don't see
it changing in the future.

Here's a minimal example of using oppai in lib mode (no error checking):

```c++
#include "../main.cc"

// don't forget to define OPPAI_LIB=1 in your build script!

#define BUFSIZE 2000000
static char buf[BUFSIZE];

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s file.osu\n", argv[0]);
        return 1;
    }

    // if you need to multithread, create one ctx and buffer for each thread
    oppai_ctx ctx;

    beatmap b(&ctx);
    beatmap::parse(argv[1], b, buf, BUFSIZE, true);

    // b.apply_mods(mods::...)

    f64 stars, aim = 0, speed = 0;

    d_calc_ctx dctx(&ctx);
    stars = d_calc(&dctx, b, &aim, &speed);

    printf(
        "%.17g stars, %.17g aim stars, %.17g speed stars\n",
        stars, aim, speed
    );

    pp_calc_result result = pp_calc(&ctx, aim, speed, b /* mods ... */);
    printf("%.17g pp\n", result.pp);

    return 0;
}
```

Recommended compiler flags for linux:

```bash
${CXX:-g++}              \
    -std=c++98           \
    -pedantic            \
    -O2                  \
    -Wno-variadic-macros \
    -Wall -Werror        \
    main.cc              \
    -lm -lstdc++         \
    -lcrypto             \
    -DOPPAI_LIB=1        \
    -o test
```

... and windows:
```batch
cl  -D_CRT_SECURE_NO_WARNINGS=1 ^
    -DNOMINMAX=1 ^
    -DOPPAI_LIB=1 ^
    -O2 ^
    -nologo -MT -Gm- -GR- -EHsc -W4 -WX ^
    -wd4201 ^
    -wd4100 ^
    -wd4458 ^
    -F8000000 ^
    main.cc ^
    -Fetest.exe ^
    Advapi32.lib
```

See lib_example/main.cc for a more complete example with error checking.
