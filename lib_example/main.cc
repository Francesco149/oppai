//#include "/path/to/oppai/code/main.cc"
#include "../main.cc"

// don't forget to define OPPAI_LIB=1 in your build script!

void print_pp(pp_calc_result &res)
{
    printf(
        "\n%.17g aim\n%.17g speed\n%.17g acc\n%.17g pp\nfor %.17g%%\n",
        res.aim_pp, res.speed_pp, res.acc_pp, res.pp, res.acc_percent
    );
}

void print_diff(f64 stars, f64 aim, f64 speed)
{
    printf(
        "\n%.17g stars\n%.17g aim stars\n%.17g speed stars\n",
        stars, aim, speed
    );
}

// should be big enough to hold your osu file
#define BUFSIZE 2000000
static char buf[BUFSIZE];

void check_err(oppai_ctx* ctx)
{
    if (oppai_err(ctx)) {
        fprintf(stderr, "%s\n", oppai_err(ctx));
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s file.osu\n", argv[0]);
        return 1;
    }

    // if you need to multithread, create one ctx and buffer for each thread
    oppai_ctx ctx;

    // parse beatmap -----------------------------------------------------------
    beatmap b(&ctx);
    beatmap::parse(argv[1], b, buf, BUFSIZE);
    // beatmaps will be cached in the executable's folder

    check_err(&ctx);

    printf(
        "%s - %s [%s] (by %s)\n"
        "CS%g OD%g AR%g HP%g\n"
        "%u objects (%u circles, %u sliders, %u spinners)\n"
        "max combo: %u\n",
        b.artist, b.title, b.version, b.creator,
        b.cs, b.od, b.ar, b.hp,
        (u32)b.num_objects,
        (u32)b.num_circles,
        (u32)b.num_sliders,
        (u32)b.num_spinners,
        (u32)b.max_combo
    );

    // diff calc ---------------------------------------------------------------
    f64 stars, aim = 0, speed = 0;

    d_calc_ctx dctx(&ctx);
    stars = d_calc(&dctx, b, &aim, &speed);

    check_err(&ctx);
    print_diff(stars, aim, speed);

    // pp calc -----------------------------------------------------------------
    pp_calc_result res =
        pp_calc(&ctx, aim, speed, b);

    check_err(&ctx);
    print_pp(res);

    // pp calc (with acc %) ----------------------------------------------------
    res = pp_calc_acc(&ctx, aim, speed, b, 90.0);
    check_err(&ctx);

    print_pp(res);

    // override OD example -----------------------------------------------------
    printf("\n----\nIf the map was od10:\n");

    f32 old_od = b.od;
    b.od = 10;

    res = pp_calc(&ctx, aim, speed, b);
    check_err(&ctx);

    print_pp(res);

    b.od = old_od;

    // override AR example -----------------------------------------------------
    printf("\n----\nIf the map was ar11:\n");

    f32 old_ar = b.ar;
    b.ar = 11;

    res = pp_calc(&ctx, aim, speed, b);
    check_err(&ctx);

    print_pp(res);

    b.ar = old_ar;

    // override CS example -----------------------------------------------------
    printf("\n----\nIf the map was cs6.5:\n");

    f32 old_cs = b.cs;
    b.cs = 6.5;

    // remember that CS is map-changing so difficulty must be recomputed
    stars = d_calc(&dctx, b, &aim, &speed);
    check_err(&ctx);

    print_diff(stars, aim, speed);

    res = pp_calc(&ctx, aim, speed, b);
    check_err(&ctx);

    print_pp(res);

    b.cs = old_cs;

    // mods example ------------------------------------------------------------
    printf("\n----\nWith HDHR:\n");

    // mods are a bitmask, same as what the osu! api uses
    u32 mods = mods::hd | mods::hr;
    b.apply_mods(mods);

    // mods are map-changing, recompute diff
    stars = d_calc(&dctx, b, &aim, &speed);
    check_err(&ctx);

    print_diff(stars, aim, speed);

    res = pp_calc(&ctx, aim, speed, b, mods);
    check_err(&ctx);

    print_pp(res);

    return 0;
}
