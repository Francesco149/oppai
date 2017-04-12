//#include "/path/to/oppai/code/main.cc"
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

    if (oppai_err(&ctx)) {
        fprintf(stderr, "parse: %s\n", oppai_err(&ctx));
        return 1;
    }

    // b.apply_mods(mods::...)

    f64 stars, aim = 0, speed = 0;

    d_calc_ctx dctx(&ctx);
    stars = d_calc(&dctx, b, &aim, &speed);

    if (oppai_err(&ctx)) {
        fprintf(stderr, "d_calc: %s\n", oppai_err(&ctx));
        return 1;
    }

    printf(
        "%.17g stars, %.17g aim stars, %.17g speed stars\n",
        stars, aim, speed
    );

    pp_calc_result result = pp_calc(&ctx, aim, speed, b /* mods ... */);
    printf("%.17g pp\n", result.pp);

    return 0;
}
