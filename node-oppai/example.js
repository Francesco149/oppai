const path = require('path');
const util = require('util');
const oppai = require('oppai');

function print_pp(res)
{
    console.log(
        util.format(
            "\n%d aim\n%d speed\n%d acc\n%d pp\nfor %d%%",
            res.aimPp, res.speedPp, res.accPp, res.pp, res.accPercent
        )
    );
}

function print_diff(diff)
{
    console.log(
        util.format(
            "\n%d stars\n%d aim stars\n%d speed stars",
            diff.stars, diff.aim, diff.speed
        )
    );
}

function chk(ctx)
{
    if (ctx.err()) {
        console.error(ctx.err());
        process.exit(1);
    }
}

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

    print_diff(diff);

    // pp calc -----------------------------------------------------------------
    res = ctx.ppCalc(diff.aim, diff.speed, b);
    chk(ctx);

    print_pp(res);

    // pp calc (with acc %) ----------------------------------------------------
    res = ctx.ppCalcAcc(diff.aim, diff.speed, b, 90.0);
    chk(ctx);

    print_pp(res);

    // override OD example -----------------------------------------------------
    console.log("\n----\nIf the map was od10:");
    var oldOd = b.od();
    b.setOd(10);

    res = ctx.ppCalc(diff.aim, diff.speed, b);
    chk(ctx);

    print_pp(res);

    b.setOd(oldOd);

    // override AR example -----------------------------------------------------
    console.log("\n----\nIf the map was ar11:");
    var oldAr = b.ar();
    b.setAr(11);

    res = ctx.ppCalc(diff.aim, diff.speed, b);
    chk(ctx);

    print_pp(res);

    b.setAr(oldAr);

    // override CS example -----------------------------------------------------
    console.log("\n----\nIf the map was cs6.5:");
    var oldCs = b.cs();
    b.setCs(6.5);

    // remember that CS is map-changing so difficulty must be recomputed
    diff = dctx.diffCalc(b);
    chk(ctx);
    print_diff(diff);

    res = ctx.ppCalc(diff.aim, diff.speed, b);
    chk(ctx);

    print_pp(res);

    b.setCs(oldCs);

    // mods example ------------------------------------------------------------
    console.log("\n----\nWith HDHR:");

    var mods = oppai.hd | oppai.hr;
    b.applyMods(mods);

    // mods are map-changing, recompute diff
    diff = dctx.diffCalc(b);
    chk(ctx);
    print_diff(diff);

    res = ctx.ppCalc(diff.aim, diff.speed, b, mods);
    chk(ctx);

    print_pp(res);
}

main();
