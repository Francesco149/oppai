#include "types.hh"
#include "common.hh"

// at the moment I'm not using static anywhere since it's a monolithic build
// anyways. it might be nice to have it though, for extra scope safety

// common includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <cmath>

#include <ctype.h> // tolower/toupper

const char* version_string = "0.8.6";

// -----------------------------------------------------------------------------

// sets the last error to msg if it's not already set
#define die(msg) dbgputs(msg); die_impl(msg)

const char* last_err = 0;

void die_impl(const char* msg) {
    if (last_err) {
        return;
    }

    last_err = msg;
}

// returns the last error, or 0 if no error has occurred
const char* err() {
    return last_err;
}

void chk() {
    if (!err()) {
        return;
    }

    fputs(err(), stderr);
    fputs("\n", stderr);
    exit(1);
}

// -----------------------------------------------------------------------------

// these don't necessarily need to match the pp processor's bitmask 1:1 but
// I'll keep them consistent just because
namespace mods {
    const u32
        nomod = 0,
        nf = 1 << 0,
        ez = 1 << 1,
        hd = 1 << 3,
        hr = 1 << 4,
        dt = 1 << 6,
        ht = 1 << 8,
        nc = 1 << 9,
        fl = 1 << 10,
        so = 1 << 12,
        speed_changing = dt | ht | nc,
        map_changing = hr | ez | speed_changing;
}

const char* const mod_strs[] = {
    "nomod", "nf", "ez", "hd", "hr", "dt", "ht", "nc", "fl", "so"
};
const u32 mod_masks[] = {
    mods::nomod, mods::nf, mods::ez, mods::hd, mods::hr, mods::dt, mods::ht,
    mods::nc, mods::fl, mods::so
};
const size_t num_mods = sizeof(mod_masks) / sizeof(mod_masks[0]);

// -----------------------------------------------------------------------------

// TODO: move this
bool encode_str(FILE* fd, const char* str);

// this a monolithic build. stuff is separated into files purely for readability
#include "profiler.cc"
#include "math.cc"
#include "beatmap.cc"
#include "diff_calc.cc"
#include "pp_calc.cc"

// -----------------------------------------------------------------------------

// globals for main
beatmap b;

#ifdef SHOW_BEATMAP
void print_beatmap();
#endif

// -----------------------------------------------------------------------------

// output modules
#define print_sig(name)                 \
    void name(                          \
        char*       mods_str,           \
        u16         combo,              \
        u16         misses,             \
        u32         scoring,            \
        f64         stars,              \
        f64         aim,                \
        f64         speed,              \
        f64         rhythm_awkwardness, \
        u16         nsingles,           \
        u16         nsingles_timing,    \
        u16         nsingles_threshold, \
        pp_calc_result& res)

// text output
print_sig(text_print) {
    // round to 2 decimal places
    aim = macro_round(aim * 100.0) / 100.0;
    speed = macro_round(speed * 100.0) / 100.0;
    stars = macro_round(stars * 100.0) / 100.0;
    res.pp = macro_round(res.pp * 100.0) / 100.0;
    res.pp = macro_round(res.aim_pp * 100.0) / 100.0;
    res.pp = macro_round(res.speed_pp * 100.0) / 100.0;
    res.pp = macro_round(res.acc_pp * 100.0) / 100.0;

    printf("o p p a i | v%s\n", version_string);
    puts("s     d n | ");
    puts("u     v s | (looking for");
    puts("!     a p | cool ascii");
    puts("      n e | to put here)");
    puts("      c c | ");
    puts("      e t | ");
    puts("      d o | ");
    puts("        r |\n");

    printf("\n%s - %s [%s] (%s) %s\n",
            b.artist, b.title, b.version, b.creator, mods_str ? mods_str : "");

    printf("od%g ar%g cs%g hp%g\n", b.od, b.ar, b.cs, b.hp);
    printf("%" fu16 "/%" fu16 " combo\n", combo, b.max_combo);
    printf("%" fu16 " circles, %" fu16 " sliders %" fu16 " spinners\n",
            b.num_circles, b.num_sliders, b.num_spinners);
    printf("%" fu16 "xmiss\n", misses);
    printf("%g%%\n", res.acc_percent);
    printf("scorev%" fu32"\n\n", scoring);

    printf("%g stars\naim stars: %g, speed stars: %g\n\n", stars, aim, speed);

    printf("aim: %g\n", res.aim_pp);
    printf("speed: %g\n", res.speed_pp);
    printf("accuracy: %g\n", res.acc_pp);

    printf("\nrhythm awkwardness (beta): %g\n", rhythm_awkwardness);

    f64 awkwardness_bonus =
        std::max(1.0, std::min(1.15, std::pow(rhythm_awkwardness, 0.3)));
    printf("awkwardness acc pp bonus (beta): %g\n", awkwardness_bonus);

    printf("%" fu16 " spacing singletaps (%g%%)\n",
           nsingles, (f32)nsingles / (b.num_circles + b.num_sliders) * 100.0f);

    printf("%" fu16 " timing singletaps (%g%%)\n",
           nsingles_timing,
           (f32)nsingles_timing / (b.num_circles + b.num_sliders) * 100.0f);

    printf("%" fu16 " notes within singletap threshold (%g%%)\n",
           nsingles_threshold,
           (f32)nsingles_threshold / (b.num_circles + b.num_sliders) * 100.0f);

    printf("\n%gpp\n", res.pp);
}

// json output
void print_escaped_json_string(const char* str) {
    putchar('"');

    const char* chars_to_escape = "\\\"";
    for (; *str; ++str) {
        // escape all characters in chars_to_escape
        for (const char* p = chars_to_escape; *p; ++p) {
            if (*p == *str) {
                putchar('\\');
            }
        }

        putchar(*str);
    }

    putchar('"');
}

print_sig(json_print) {
    printf("{\"oppai_version\":");
    print_escaped_json_string(version_string);

    // first print the artist, title, version and creator like this
    // since json-string so " and \ needs to be escaped
    printf(",\"artist\":");
    print_escaped_json_string(b.artist);
    printf(",\"title\":");
    print_escaped_json_string(b.title);
    printf(",\"version\":");
    print_escaped_json_string(b.version);
    printf(",\"creator\":");
    print_escaped_json_string(b.creator);

    // now print the rest
    printf(
        ","
        "\"mods_str\": \"%s\","
        "\"od\":%.2g,\"ar\":%.2g,\"cs\":%.2g,\"hp\":%.2g,"
        "\"combo\": %" fu16 ",\"max_combo\": %" fu16 ","
        "\"num_circles\": %" fu16 ","
        "\"num_sliders\": %" fu16 ","
        "\"num_spinners\": %" fu16 ","
        "\"misses\": %" fu16 ","
        "\"score_version\": %" fu32 ","
        "\"stars\": %.17g,\"speed_stars\": %.17g,\"aim_stars\": %.17g,"
        "\"rhythm_awkwardness\": %.17g,"
        "\"nsingles\": %" fu16 ","
        "\"nsingles_timing\": %" fu16 ","
        "\"nsingles_threshold\": %" fu16 ","
        "\"pp\":%.17g"
        "}\n",
        mods_str ? mods_str : "",
        b.od, b.ar, b.cs, b.hp,
        combo, b.max_combo,
        b.num_circles, b.num_sliders, b.num_spinners,
        misses, scoring,
        stars, speed, aim,
        rhythm_awkwardness,
        nsingles, nsingles_timing, nsingles_threshold,
        res.pp
    );
}

// binary output
bool encode_str(FILE* fd, const char* str)
{
    if (strlen(str) > 0xFFFF) {
        die("encode_str can only handle strings up to FFFF characters");
        return false;
    }

    u16 len = (u16)strlen(str);

    if (fwrite(&len, 2, 1, fd) != 1) {
        perror("fwrite");
        return false;
    }

    if (fwrite(str, 1, len, fd) != len) {
        perror("fwrite");
        return false;
    }

    return true;
}

// binary format history:
// version 2: added hp

print_sig(binary_print) {
    if (!freopen(0, "wb", stdout)) {
        perror(0);
        exit(1);
    }

    u32 binary_output_version = 2;

    // TODO: shorten this with macros
    putc('\0', stdout);
    putc('\0', stdout); // is_struct
    fwrite(&binary_output_version, 4, 1, stdout);
    encode_str(stdout, version_string); chk();
    encode_str(stdout, b.artist); chk();
    encode_str(stdout, b.title); chk();
    encode_str(stdout, b.version); chk();
    encode_str(stdout, b.creator); chk();
    encode_str(stdout, mods_str ? mods_str : ""); chk();
    fwrite(&b.od, sizeof(f32), 1, stdout);
    fwrite(&b.ar, sizeof(f32), 1, stdout);
    fwrite(&b.cs, sizeof(f32), 1, stdout);
    fwrite(&b.hp, sizeof(f32), 1, stdout);
    fwrite(&combo, 2, 1, stdout);
    fwrite(&b.max_combo, 2, 1, stdout);
    fwrite(&b.num_circles, 2, 1, stdout);
    fwrite(&b.num_sliders, 2, 1, stdout);
    fwrite(&b.num_spinners, 2, 1, stdout);
    fwrite(&misses, 2, 1, stdout);
    fwrite(&scoring, 4, 1, stdout);

    f32 tmp = (f32)stars;
    fwrite(&tmp, sizeof(f32), 1, stdout);

    tmp = (f32)speed;
    fwrite(&tmp, sizeof(f32), 1, stdout);

    tmp = (f32)aim;
    fwrite(&tmp, sizeof(f32), 1, stdout);

    tmp = (f32)res.pp;
    fwrite(&tmp, sizeof(f32), 1, stdout);
}

#ifndef __GNUC__
#pragma pack(push, 1)
#endif

// binary struct output
struct binary_output_data {
    u8 must_be_zero;
    u8 is_struct;
    u32 output_version;
    char oppai_version[256];
    char artist[256];
    char title[256];
    char version[256];
    char creator[256];
    char mods_str[256];
    f32 od, ar, cs, hp;
    u16 combo, max_combo;
    u16 num_circles, num_sliders, num_spinners;
    u16 misses;
    u32 scoring;
    f32 stars, speed, aim;
    f32 pp;
}
#ifdef __GNUC__
__attribute__ ((aligned (1), packed));
#else
;
#pragma pack(pop)
#endif

print_sig(binary_struct_print) {
    if (!freopen(0, "wb", stdout)) {
        perror(0);
        exit(1);
    }

    binary_output_data d = {0};

    d.is_struct = 1;
    d.output_version = 1;
    strcpy(d.oppai_version, version_string);
    strcpy(d.artist, b.artist);
    strcpy(d.title, b.title);
    strcpy(d.version, b.version);
    strcpy(d.creator, b.creator);
    strcpy(d.mods_str, mods_str ? mods_str : "");
    d.od = b.od;
    d.ar = b.ar;
    d.cs = b.cs;
    d.hp = b.hp;
    d.combo = combo;
    d.max_combo = b.max_combo;
    d.num_circles = b.num_circles;
    d.num_sliders = b.num_sliders;
    d.num_spinners = b.num_spinners;
    d.misses = misses;
    d.scoring = scoring;
    d.stars = (f32)stars;
    d.speed = (f32)speed;
    d.aim = (f32)aim;
    d.pp = (f32)res.pp;

    fwrite(&d, sizeof(binary_output_data), 1, stdout);
}

// ---

typedef print_sig(print_callback);
#undef print_sig

struct output_module {
    const char* name;
    print_callback* print;
};

output_module modules[] = {
    { "text", text_print },
    { "json", json_print },
    { "binary", binary_print },
    { "binary_struct", binary_struct_print },
    { 0, 0 }
};

output_module* get_output_module(const char* name) {
    for (output_module* m = modules; m->name; ++m) {
        if (!strcmp(m->name, name)) {
            return m;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    // TODO: abstract error outputting into the output modules
    if (argc < 2) {
        printf("Usage: %s /path/to/difficulty.osu "
                "{[acc]%% or [num_100s]x100 [num_50s]x50} +[mods] "
                "[combo]x [misses]m scorev[scoring_version] "
                "AR[ar_override] OD[od_override] CS[cs_override] "
                "-st[singletap_max_bpm] "
                "-o[output_module] [-no-awkwardness]\n\n", *argv);
        puts("use \"-\" instead of a path to an .osu file to read from stdin");
        puts("acc: the accuracy in percent (example: 99.99%)");
        puts("num_100s, num_50s: used to specify accuracy in 100 and 50 count");
        puts("mods: any combination of nomod, nf, ez, hd, hr, dt, ht"
                ", nc, fl, so (example: +hddthr)");
        puts("combo: the highest combo (example: 1337x)");
        puts("misses: amount of misses (example: 1m)");
        puts("scoring_version: can only be 1 or 2 (example: scorev2)");
        puts("ar_override, od_override, cs_override: overrides base ar/od/cs "
             "for the map. useful to quickly calculate how it would affect pp "
             "without editing the actual map. (example: AR10 OD10 CS6.5)");
        puts("singletap_max_bpm: singletap bpm threshold, defaults to 240. "
             "this is used to count how many notes are within 1/2 singletaps "
             "of this bpm.");

        printf("output_module: the module that will be used to output the "
             "results (defaults to text). currently available modules: ");

        for (output_module* m = modules; m->name; ++m)
        {
            printf("%s ", m->name);
        }

        puts("");

        puts("-no-awkwardness: disables rhythm awkwardness calculation "
               "for speed (about 10% performance gain)");

        puts("\narguments in [square brackets] are optional");
        puts("(the order of the optional arguments does not matter)");

        return 1;
    }

#if OPPAI_PROFILING
    const int prid = 0;
#endif

    bool no_cache = false;

    // TODO: find a way to do this without using 2 loops
    for (int i = 2; i < argc; i++)
    {
        // no cache
        if (!strcmp(argv[i], "-no-cache")) {
            no_cache = true;
            argv[i] = 0;
            break;
        }
    }

    profile_init();

    profile(prid, "beatmap parse");
    beatmap::parse(argv[1], b, no_cache);
    chk();

    profile(prid, "arguments parse");
    char* output_module_name = (char*)"text";
    char* mods_str = 0;
    f64 acc = 0;
    f32 ar, od, cs;
    u32 mods = mods::nomod;
    u16 combo = b.max_combo;
    u16 misses = 0;
    u32 scoring = 1;
    u16 c100 = 0, c50 = 0;
    i32 single_max_bpm = 240;

    // TODO: bitmask
    bool no_percent = true;
    bool no_awkwardness = false;

    dbgputs("\nparsing arguments");
    for (int i = 2; i < argc; i++) {
        char suff[64] = {0};
        char* a = argv[i];

        if (!a) {
            continue;
        }

        std::transform(a, a + strlen(a), a, tolower);

        if (a[0] == '-')
        {
            // output module
            if (a[1] == 'o') {
                output_module_name = a + 2;
                continue;
            }

            // singletap threshold
            i32 tmp;
            if (sscanf(a + 1, "st%" fi32, &tmp) == 1) {
                single_max_bpm = tmp;
                continue;
            }

            // no rhythm awkwardness
            if (!strcmp(a + 1, "no-awkwardness")) {
                no_awkwardness = true;
                continue;
            }
        }

        // acc
        f64 tmp_acc;
        if (sscanf(a, "%lf%s", &tmp_acc, suff) == 2 && !strcmp(suff, "%")) {
            acc = tmp_acc;
            no_percent = false;
            continue;
        }

        // 100s, 50s
        u16 tmp_c100 = 0, tmp_c50 = 0;
        if (sscanf(a, "%" fu16 "%s", &tmp_c100, suff) == 2 &&
            !strcmp(suff, "x100")) {

            c100 = tmp_c100;
            continue;
        }

        if (sscanf(a, "%" fu16 "%s", &tmp_c50, suff) == 2 &&
            !strcmp(suff, "x50")) {

            c50 = tmp_c50;
            continue;
        }

        // mods
        char* tmp_mods_str = 0;
        for (size_t j = 0; j < num_mods; j++) {
            if (strstr(a, mod_strs[j])) {
                tmp_mods_str = a;
                mods |= mod_masks[j];
            }
        }

        if (tmp_mods_str == a && *tmp_mods_str == '+') {
            // at least one mod found in the parameter and the prefix matches
            mods_str = tmp_mods_str;
            std::transform(mods_str, mods_str + strlen(mods_str), mods_str,
                    toupper);
            continue;
        }

        // combo
        u16 tmp_combo;
        if (sscanf(a, "%" fu16 "%s", &tmp_combo, suff) == 2 &&
            !strcmp(suff, "x")) {

            combo = tmp_combo;
            continue;
        }

        // misses
        u16 tmp_misses;
        if (sscanf(a, "%" fu16 "%s", &tmp_misses, suff) == 2 &&
                (!strcmp(suff, "xm") || !strcmp(suff, "xmiss") ||
                 !strcmp(suff, "m"))) {
            misses = tmp_misses;
            continue;
        }

        // scorev1 / scorev2
        u32 tmp_scoring;
        if (sscanf(a, "scorev%" fu32, &tmp_scoring) == 1) {
            scoring = tmp_scoring;
            continue;
        }

        // AR/OD/CS override
        if (sscanf(a, "ar%f", &ar) == 1) {
            b.ar = ar;
            continue;
        }

        if (sscanf(a, "od%f", &od) == 1) {
            b.od = od;
            continue;
        }

        if (sscanf(a, "cs%f", &cs) == 1) {
            b.cs = cs;
            continue;
        }

        printf(">%s\n", a);
        die("Invalid parameter");
        break;
    }

    chk();

    // ---

#ifdef SHOW_BEATMAP
    print_beatmap();
    chk();
#endif

    profile(prid, "diff calc");
    b.apply_mods(mods);
    chk();

    u16 nsingles = 0, nsingles_timing = 0, nsingles_threshold = 0;
    f64 aim, speed, rhythm_complexity = 0;
    f64 stars =
        d_calc(
            b,
            &aim,
            &speed,
            no_awkwardness ? 0 : &rhythm_complexity,
            &nsingles,
            &nsingles_timing,
            &nsingles_threshold,
            (i32)((60000.0f / single_max_bpm) / 2)
        );
    chk();

    pp_calc_result res = no_percent ?
        pp_calc(aim, speed, b, mods, combo, misses, 0xFFFF, c100, c50, scoring)
        : pp_calc_acc(aim, speed, b, acc, mods, combo, misses, scoring);

    chk();

    // ---

    profile(prid, "output");
    output_module* m = get_output_module(output_module_name);
    if (!m) {
        die("The specified output module does not exist");
    }
    chk();

    m->print(mods_str, combo, misses, scoring,
             stars, aim, speed, rhythm_complexity,
             nsingles, nsingles_timing, nsingles_threshold, res);

    // ---

    profile(prid, "");

    profile_end();

    return 0;
}

#ifdef SHOW_BEATMAP
void print_beatmap() {
    printf(
        "Format version: %" fi32 "\n"
        "Stack Leniency: %g\n"
        "Mode: %" fi32 "\n"
        "Title: %s\n"
        "Artist: %s\n"
        "Version: %s\n"
        "HP%g CS%g OD%g AR%g SV%g\n\n"
        ,
        b.format_version,
        b.stack_leniency,
        b.mode,
        b.title,
        b.artist,
        b.version,
        b.hp, b.cs, b.od, b.ar, b.sv
    );

    printf("> %" fu32 " timing points\n", (u32)b.num_timing_points);

    for (size_t i = 0; i < b.num_timing_points; i++)
    {
        timing_point& tp = b.timing_points[i];
        printf("%" fi32 ": ", tp.time);

        if (!tp.inherit) {
            printf("%g bpm\n", 60000.0 / tp.ms_per_beat);
        } else {
            printf("%gx\n", -100.0 / tp.ms_per_beat);
        }
    }

    printf("\n> %" fu32 " hit objects\n", (u32)b.num_objects);

    for (size_t i = 0; i < b.num_objects; i++) {

        hit_object& ho = b.objects[i];
        switch (ho.type) {
            case obj::circle:
                printf("%" fi32 ": Circle (%g, %g)\n",
                    ho.time, ho.pos.x, ho.pos.y);
                break;

            case obj::spinner:
                printf("%" fi32 "-%" fi32 ": Spinner\n",
                        ho.time, ho.end_time);
                break;

            case obj::slider:
            {
                slider_data& sl = ho.slider;

                printf(
                    "%" fi32 "-%" fi32 ": Slider "
                    "[Type %c, Length %g, %" fu16 " Repetitions] ",
                    ho.time, ho.end_time, sl.type,
                    sl.length, sl.repetitions);

                puts("");

                break;
            }

            default:
                die("Invalid object type");
                return;
        }
    }
}
#endif

