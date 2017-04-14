Python bindings for oppai. Tested on windows and linux.

Supports Python 2.7 and 3+.

# What is this and why should I use it
Bindings are much cleaner and high-performance than spawning an oppai process,
especially if you need to run calculations on thousands of maps.
It also makes it easier to handle errors and customize behaviour beyond what
the command line tool can do.

# How to install
Make sure you have git to clone this repository. On linux it's usually available
in your package manager if not already installed, while on windows you will need
to install git bash.

```
git clone https://github.com/Francesco149/oppai.git
cd oppai/pyoppai
python setup.py install
```

(use sudo for the last command if you're on Linux).

# Documentation
Start the python interpreter and run the following to see the full documentation

```python
import pyoppai

help(pyoppai)
```

# Usage
Here's a minimal example of parsing a beatmap, calculating difficulty and
calculating pp. No error checking for brevity. See ```example.py``` for full
error checking and more advanced features.

```python
import sys
import os
import pyoppai

def main():
    if len(sys.argv) != 2:
        print("Usage: " + sys.argv[0] + " file.osu")
        sys.exit(1)

    ctx = pyoppai.new_ctx()

    # parse beatmap ------------------------------------------------------------
    b = pyoppai.new_beatmap(ctx)

    BUFSIZE = 2000000 # should be big enough to hold the .osu file
    buf = pyoppai.new_buffer(BUFSIZE)

    pyoppai.parse(
        sys.argv[1],
        b,
        buf,
        BUFSIZE,

        # don't disable caching and use python script's folder for caching
        False,
        os.path.dirname(os.path.realpath(__file__))
    );

    # diff calc ----------------------------------------------------------------
    dctx = pyoppai.new_d_calc_ctx(ctx)

    stars, aim, speed, _, _, _, _ = pyoppai.d_calc(dctx, b)

    print(
        "%.17g stars\n%.17g aim stars\n%.17g speed stars" %
        (stars, aim, speed)
    )

    # pp calc ------------------------------------------------------------------
    acc, pp, aim_pp, speed_pp, acc_pp = \
            pyoppai.pp_calc(ctx, aim, speed, b)

    print(
        "\n%.17g aim\n%.17g speed\n%.17g acc\n%.17g pp\nfor %.17g%%" %
            (aim_pp, speed_pp, acc_pp, pp, acc)
    )

main()
```
