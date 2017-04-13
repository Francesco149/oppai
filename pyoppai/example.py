import sys
import os
import pyoppai

def print_pp(acc, pp, aim_pp, speed_pp, acc_pp):
    print(
        "\n%.17g aim\n%.17g speed\n%.17g acc\n%.17g pp\nfor %.17g%%" %
            (aim_pp, speed_pp, acc_pp, pp, acc)
    )

def print_diff(stars, aim, speed):
    print(
        "\n%.17g stars\n%.17g aim stars\n%.17g speed stars" %
        (stars, aim, speed)
    )

def chk(ctx):
    err = pyoppai.err(ctx)

    if err:
        print err
        sys.exit(1)

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

    chk(ctx)

    print("Cache folder: " + os.path.dirname(os.path.realpath(__file__)) + "\n")

    cs, od, ar, hp = pyoppai.stats(b)

    print(
        "%s - %s [%s] (by %s)\n"
        "CS%g OD%g AR%g HP%g\n"
        "%d objects (%d circles, %d sliders, %d spinners)\n"
        "max combo: %d" %
        (
            pyoppai.artist(b),
            pyoppai.title(b),
            pyoppai.version(b),
            pyoppai.creator(b),
            cs, od, ar, hp,
            pyoppai.num_objects(b),
            pyoppai.num_circles(b),
            pyoppai.num_sliders(b),
            pyoppai.num_spinners(b),
            pyoppai.max_combo(b)
        )
    )

    # diff calc ----------------------------------------------------------------
    dctx = pyoppai.new_d_calc_ctx(ctx)

    stars, aim, speed, _, _, _, _ = pyoppai.d_calc(dctx, b)
    chk(ctx)

    print_diff(stars, aim, speed)

    # pp calc ------------------------------------------------------------------
    acc, pp, aim_pp, speed_pp, acc_pp = \
            pyoppai.pp_calc(ctx, aim, speed, b)

    chk(ctx)

    print_pp(acc, pp, aim_pp, speed_pp, acc_pp)

    # pp calc (with acc %) -----------------------------------------------------
    acc, pp, aim_pp, speed_pp, acc_pp = \
        pyoppai.pp_calc_acc(ctx, aim, speed, b, 90.0)

    chk(ctx)

    print_pp(acc, pp, aim_pp, speed_pp, acc_pp)

    # override OD example ------------------------------------------------------
    print("\n----\nIf the map was od10:")
    pyoppai.set_od(b, 10)

    acc, pp, aim_pp, speed_pp, acc_pp = \
            pyoppai.pp_calc(ctx, aim, speed, b)

    chk(ctx)

    print_pp(acc, pp, aim_pp, speed_pp, acc_pp)

    pyoppai.set_od(b, od)

    # override AR example ------------------------------------------------------
    print("\n----\nIf the map was ar11:")
    pyoppai.set_ar(b, 11)

    acc, pp, aim_pp, speed_pp, acc_pp = \
            pyoppai.pp_calc(ctx, aim, speed, b)

    chk(ctx)

    print_pp(acc, pp, aim_pp, speed_pp, acc_pp)

    pyoppai.set_ar(b, ar)

    # override CS example ------------------------------------------------------
    print("\n----\nIf the map was cs6.5:")
    pyoppai.set_cs(b, 6.5)

    # remember that CS is map-changing so difficulty must be recomputed
    stars, aim, speed, _, _, _, _ = pyoppai.d_calc(dctx, b)
    print_diff(stars, aim, speed)

    acc, pp, aim_pp, speed_pp, acc_pp = \
            pyoppai.pp_calc(ctx, aim, speed, b)

    chk(ctx)

    print_pp(acc, pp, aim_pp, speed_pp, acc_pp)

    pyoppai.set_cs(b, cs)

    # mods example -------------------------------------------------------------
    print("\n----\nWith HDHR:")

    # mods are a bitmask, same as what the osu! api uses
    mods = pyoppai.hd | pyoppai.hr
    pyoppai.apply_mods(b, mods)

    # mods are map-changing, recompute diff
    stars, aim, speed, _, _, _, _ = pyoppai.d_calc(dctx, b)
    print_diff(stars, aim, speed)

    acc, pp, aim_pp, speed_pp, acc_pp = \
            pyoppai.pp_calc(ctx, aim, speed, b, mods)

    chk(ctx)

    print_pp(acc, pp, aim_pp, speed_pp, acc_pp)

main()
