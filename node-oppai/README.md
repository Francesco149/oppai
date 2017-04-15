Node bindings for oppai, a ppv2 and difficulty calculator for osu!.

Tested on node v7.0.0 on linux, should work on windows as long as you have a
compiler that npm can use. At some point I'll include pre-built windows
binaries.

# Installing
```bash
mkdir myproject
cd myproject
npm install oppai
# write your index.js
```

If you're on windows, you might need to set up build tools:

```bash
npm install --global --production windows-build-tools
```

If you get compilation errors, try updating node to at least the latest LTS
version (I use 7.0.0).

If it's still broken, hit me up on github issues and specify your operating
system, ```node -v``` and full output of ```npm install oppai```.

# Usage
Here's a rather minimal example that parses a beatmap and calculates difficulty
and pp.

You can find full documentation here:
[Markdown Documentation](https://github.com/Francesco149/oppai/blob/master/node-oppai/docs/README.md).

```javascript
const path = require('path');
const util = require('util');
const oppai = require('oppai');

function main()
{
    process.argv.splice(0, 1);

    if (process.argv.length != 2)
    {
        console.error("Usage: " + process.argv[0] + " file.osu");
        process.exit(1);
    }

    var script_path = path.dirname(process.argv[0]);

    // if you need to multithread, create one ctx and buffer for each thread
    var ctx = oppai.Ctx();

    // parse beatmap -----------------------------------------------------------
    var b = oppai.Beatmap(ctx);

    const BUFSIZE = 2000000; // should be big enough to hold the .osu file
    var buf = oppai.Buffer(BUFSIZE);

    b.parse(
        process.argv[1],
        buf,
        BUFSIZE,

        // don't disable caching and use js script's folder for caching
        false,
        script_path
    );

    chk(ctx);

    console.log("Cache folder: " + script_path + "\n");

    console.log(
        util.format(
            "%s - %s [%s] (by %s)\n" +
            "CS%d OD%d AR%d HP%d\n" +
            "%d objects (%d circles, %d sliders, %d spinners)\n" +
            "max combo: %d",
            b.artist(), b.title(), b.version(), b.creator(),
            b.cs().toPrecision(2), b.od().toPrecision(2),
            b.ar().toPrecision(2), b.hp().toPrecision(2),
            b.numObjects(), b.numCircles(), b.numSliders(), b.numSpinners(),
            b.maxCombo()
        )
    );

    // diff calc ---------------------------------------------------------------
    dctx = oppai.DiffCalcCtx(ctx);

    diff = dctx.diffCalc(b);
    chk(ctx);

    console.log(
        util.format(
            "\n%d aim\n%d speed\n%d acc\n%d pp\nfor %d%%",
            res.aimPp, res.speedPp, res.accPp, res.pp, res.accPercent
        )
    );

    // pp calc -----------------------------------------------------------------
    res = ctx.ppCalc(diff.aim, diff.speed, b);
    chk(ctx);

    console.log(
        util.format(
            "\n%d stars\n%d aim stars\n%d speed stars",
            diff.stars, diff.aim, diff.speed
        )
    )
}

main();
```
