#include <string>
#include <openssl/md5.h>

#ifdef _MSC_VER
#include <Windows.h>
#include <direct.h>
#define mkdir _mkdir

i64 get_exe_path(char* buf, i64 bufsize) {
    return GetModuleFileNameA(0, buf, (DWORD)bufsize);
}
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define mkdir(x) mkdir(x, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

i64 get_exe_path(char* buf, i64 bufsize)
{
    i64 res;

    res = readlink("/proc/self/exe", buf, bufsize);
    if (res >= 0) {
        return res;
    }

    res = readlink("/proc/curproc/file", buf, bufsize);
    if (res >= 0) {
        return res;
    }

    res = readlink("/proc/self/path/a.out", buf, bufsize);
    if (res >= 0) {
        return res;
    }

    perror("readlink");
    strcpy(buf, ".");
    return 1;
}
#endif

// shit code ahead! I am way too lazy to write nice code for parsers, sorry
// disclaimer: this beatmap parser is meant purely for difficulty calculation
//             and I don't support any malicious use of it

const size_t bufsize = 2000000; // 2 mb
u8 buf[bufsize];

const f32   od0_ms = 79.5,
            od10_ms = 19.5,
            ar0_ms = 1800,
            ar5_ms = 1200,
            ar10_ms = 450;

const f32   od_ms_step = 6,
            ar_ms_step1 = 120, // ar0-5
            ar_ms_step2 = 150; // ar5-10

// -----------------------------------------------------------------------------

namespace obj
{
    const u8
        invalid = 0,
        circle = 1,
        spinner = 2,
        slider = 3;
}

struct slider_data {
    char type;

    // this implementation of diff calc does not
    // actually use sliders because they are way more expensive
    // to compute and it's already accurate enough
    //std::vector<v2f> points;

    u16 repetitions; // starts at 1 for non-repeating sliders
    f64 length; // how much distance the curve travels in osu pixels

    slider_data() :
        repetitions(0),
        length(0)
    {};
};

struct hit_object {
    v2f pos;
    i64 time;
    u8 type;
    i64 end_time; // for spinners and sliders
    slider_data slider;

    hit_object() :
        pos(0),
        time(0),
        type(obj::invalid),
        end_time(0)
    {}
};

struct timing_point {
    i64 time;
    f64 ms_per_beat;
    bool inherit;

    timing_point() :
        time(0),
        ms_per_beat(0),
        inherit(false)
    {}
};

// TODO: move this
bool decode_str(FILE* fd, char* str)
{
    u16 len;

    if (fread(&len, 2, 1, fd) != 1) {
        perror("fread");
        return false;
    }

    if (fread(str, 1, len, fd) != (size_t)len) {
        perror("fread");
        return false;
    }

    str[len] = 0;

    return true;
}

//TODO: move this
bool find_fwd(char*& tok, const char* str)
{
    // skips forward until tok is the line right after
    // the one that contains str
    dbgprintf("skipping until %s", str);
    while (tok) {
        if (strstr(tok, str)) {
            tok = strtok(0, "\n");
            return true;
        }
        tok = strtok(0, "\n");
    }
    return false;
}

bool whitespace(const char* s)
{
    while (*s) {
        if (!isspace(*s)) {
            return false;
        }
        s++;
    }
    return true;
}

// note: values not required for diff calc will be omitted from this parser
// at least for now
struct beatmap {
    u32 format_version;

    // general
    f32 stack_leniency;
    u8 mode;

    // metadata
    char title[256];
    char artist[256];
    char creator[256];
    char version[256];

    // difficulty
    f32 hp;
    f32 cs;
    f32 od;
    f32 ar;
    f32 sv;
    f32 tick_rate;

    u16 num_circles;
    u16 num_sliders;
    u16 num_spinners;

    u16 max_combo;

    static const size_t max_timing_points = 0xFFFF;
    size_t num_timing_points;
    timing_point timing_points[max_timing_points];

    static const size_t max_objects = max_timing_points;
    size_t num_objects;
    hit_object objects[max_objects];

    bool write(FILE* fd)
    {
#define w(x) \
        if (fwrite(&x, sizeof(x), 1, fd) != 1) { \
            perror("fwrite"); \
            return false; \
        }

        w(format_version)
        w(stack_leniency)
        w(mode)

        if (!encode_str(fd, title) ||
            !encode_str(fd, artist) ||
            !encode_str(fd, creator) ||
            !encode_str(fd, version))
        {
            return false;
        }

        w(hp)
        w(cs)
        w(od)
        w(ar)
        w(sv)
        w(tick_rate)

        w(num_circles)
        w(num_sliders)
        w(num_spinners)

        w(max_combo)

        w(num_timing_points)
        w(num_objects)

        for (u16 i = 0; i < (u16)num_timing_points; ++i) {
            w(timing_points[i])
        }

        for (u16 i = 0; i < (u16)num_objects; ++i) {
            w(objects[i])
        }
#undef w

        return true;
    }

    bool read(FILE* fd)
    {
#define w(x) \
        if (fread(&x, sizeof(x), 1, fd) != 1) { \
            perror("fread"); \
            return false; \
        }

        w(format_version)
        w(stack_leniency)
        w(mode)

        if (!decode_str(fd, title) ||
            !decode_str(fd, artist) ||
            !decode_str(fd, creator) ||
            !decode_str(fd, version))
        {
            return false;
        }

        w(hp)
        w(cs)
        w(od)
        w(ar)
        w(sv)
        w(tick_rate)

        w(num_circles)
        w(num_sliders)
        w(num_spinners)

        w(max_combo)

        w(num_timing_points)
        w(num_objects)

        for (u16 i = 0; i < (u16)num_timing_points; ++i) {
            w(timing_points[i])
        }

        for (u16 i = 0; i < (u16)num_objects; ++i) {
            w(objects[i])
        }
#undef w

        return true;
    }

    beatmap() :
        format_version(0),
        stack_leniency(0),
        mode(0),
        hp(1337), cs(1337), od(1337), ar(1337),
        sv(1337), tick_rate(1),
        num_circles(0),
        num_sliders(0),
        num_spinners(0),
        max_combo(0),
        num_timing_points(0),
        num_objects(0)
    {
        memset(title, 0, sizeof(title));
        memset(artist, 0, sizeof(artist));
        memset(creator, 0, sizeof(creator));
        memset(version, 0, sizeof(version));
    }

    static
    i64 get_cache_file(char* cache_path, i64 bufsize, bool mk = true)
    {
        u8 digest_bytes[MD5_DIGEST_LENGTH];
        char* p = cache_path;
        char const* folder_name = "oppai_cache";

        MD5((u8*)buf, strlen((char const*)buf), digest_bytes);

        p += get_exe_path(
            p,
            bufsize - MD5_DIGEST_LENGTH * 2 - 3 - strlen(folder_name)
        );

        /* get base dir of exe */
        for (; *p != '\\' && *p != '/'; --p);

        p += sprintf(p, "/oppai_cache/");

        if (mk) {
            mkdir(cache_path);
        }

        for (u8 i = 0; i < MD5_DIGEST_LENGTH; ++i) {
            p += sprintf(p, "%02x", digest_bytes[i]);
        }

        *p = 0;

        dbgprintf("cache path: %s\n", cache_path);

        return p - cache_path;
    }

    // parse .osu file into a beatmap object
    static void parse(
        const char* osu_file,
        beatmap& b,
        bool disable_cache = false)
    {
#if OPPAI_PROFILING
        const int prid = 1;
#endif

        profile(prid, "I/O");

        // if osu_file is "-" read from stdin instead of file
        FILE* f = (strcmp(osu_file, "-") == 0) ? stdin : fopen(osu_file, "rb");
        if (!f) {
            die("Failed to open beatmap");
            return;
        }

        size_t cb = fread(buf, 1, bufsize, f);
        if (cb == bufsize) {
            die("Beatmap is too big for the internal buffer");
            return;
        }

        dbgprintf("%zd bytes\n", cb);
        fclose(f);

        // just to be safe
        buf[cb] = 0;

        // ---

        char cachefile[4096];
        FILE* cachefd = 0;

        if (!disable_cache) {
            get_cache_file(cachefile, sizeof(cachefile));
            cachefd = fopen(cachefile, "rb");
        }

        // ---

        profile(prid, "prepare lambdas");

        char* tok = strtok((char*)buf, "\n");

#define fwd() tok = strtok(0, "\n")
#define not_section() (tok && *tok != '[')

        // ---

        u32 tmp_format_version = 0;

        profile(prid, "format version");

        while (not_section()) {
            if (sscanf(tok, "osu file format v%" fu32 "",
                       cachefd ? &tmp_format_version :&b.format_version) == 1) {
                break;
            }
            fwd();
        }

        if (!b.format_version) {
            dbgputs("Warning: File format version not found");
        }

        // ---

        profile(prid, "general");

        if (!find_fwd(tok, "[General]")) {
            die("Could not find General info");
            return;
        }

        // ---

        // NOTE: I could just store all properties and map them by section and name
        // but I'd rather parse only the ones I need since I'd still need to parse
        // them one by one and check for format errors.

        f32 tmp_stack_leniency = 0;
        u16 tmp_mode = 0;

        // StackLeniency and Mode are not present in older formats
        for (; not_section(); fwd()) {

            if (sscanf(tok, "StackLeniency: %f",
                       cachefd ? &tmp_stack_leniency : &b.stack_leniency) == 1) {
                continue;
            }

            if (sscanf(tok, "Mode: %" fu16 "", &tmp_mode) == 1)
            {
                if (!cachefd) {
                    b.mode = (u8)tmp_mode;
                }
                continue;
            }
        }

        // ---

        profile(prid, "metadata");

        if (!find_fwd(tok, "[Metadata]")) {
            die("Could not find Metadata");
            return;
        }

        // ---

        char tmp_title[sizeof(b.title)];
        char tmp_artist[sizeof(b.artist)];
        char tmp_creator[sizeof(b.creator)];
        char tmp_version[sizeof(b.version)];

        if (cachefd)
        {
            memset(tmp_title, 0, sizeof(tmp_title));
            memset(tmp_artist, 0, sizeof(tmp_title));
            memset(tmp_creator, 0, sizeof(tmp_title));
            memset(tmp_version, 0, sizeof(tmp_title));
        }

        for (; not_section(); fwd()) {

            // %[^\r\n] means accept all characters except \r\n
            // which means that it'll grab the string until it finds \r or \n
            if (sscanf(tok, "Title: %[^\r\n]",
                       cachefd ? tmp_title : b.title) == 1) {
                continue;
            }

            if (sscanf(tok, "Artist: %[^\r\n]",
                       cachefd ? tmp_artist : b.artist) == 1) {
                continue;
            }

            if (sscanf(tok, "Creator: %[^\r\n]",
                       cachefd ? tmp_creator : b.creator) == 1) {
                continue;
            }

            if (sscanf(tok, "Version: %[^\r\n]",
                       cachefd ? tmp_version : b.version) == 1) {
                continue;
            }
        }

        // ---

        if (cachefd)
        {
            int n = b.read(cachefd) ? 1 : -1;

            if (n != 1) {
                perror("fread");
            }

            fclose(cachefd);

            if (n == 1 &&
                !strcmp(b.title, tmp_title) &&
                !strcmp(b.artist, tmp_artist) &&
                !strcmp(b.creator, tmp_creator) &&
                !strcmp(b.version, tmp_version))
            {
                return;
            }

            dbgprintf(
                "hash collision for %s: "
                "%s - %s (%s) [%s] != %s - %s (%s) [%s]\n",
                osu_file,
                b.artist, b.title, b.creator, b.version,
                tmp_artist, tmp_title, tmp_creator, tmp_version
            );

            b.format_version = tmp_format_version;
            b.mode = tmp_mode;
            b.stack_leniency = tmp_stack_leniency;

            strcpy(b.title, tmp_title);
            strcpy(b.artist, tmp_artist);
            strcpy(b.creator, tmp_creator);
            strcpy(b.version, tmp_version);
        }

        // ---

        profile(prid, "difficulty");

        if (!find_fwd(tok, "[Difficulty]")) {
            die("Could not find Difficulty");
            return;
        }

        // ---

        for (; not_section(); fwd()) {

            if (sscanf(tok, "HPDrainRate: %f", &b.hp) == 1) {
                continue;
            }

            if (sscanf(tok, "CircleSize: %f", &b.cs) == 1) {
                continue;
            }

            if (sscanf(tok, "OverallDifficulty: %f", &b.od) == 1) {
                continue;
            }

            if (sscanf(tok, "ApproachRate: %f", &b.ar) == 1) {
                continue;
            }

            if (sscanf(tok, "SliderMultiplier: %f", &b.sv) == 1) {
                continue;
            }

            if (sscanf(tok, "SliderTickRate: %f", &b.tick_rate) == 1) {
                continue;
            }
        }

        if (b.hp > 10) {
            die("Invalid or missing HP");
            return;
        }

        if (b.cs > 10) {
            die("Invalid or missing CS");
            return;
        }

        if (b.od > 10) {
            die("Invalid or missing OD");
            return;
        }

        if (b.ar > 10) {
            dbgputs("Warning: AR not found, assuming old map and setting AR=OD");
            b.ar = b.od;
        }

        if (b.sv > 10) { // not sure what max sv is
            die("Invalid or missing SV");
            return;
        }

        // ---

        profile(prid, "timing points");

        if (!find_fwd(tok, "[TimingPoints]")) {
            die("Could not find TimingPoints");
            return;
        }

        // ---

        i32 useless;

        for (; not_section(); fwd()) {

            if (whitespace(tok)) {
                dbgputs("skipping whitespace line");
                continue;
            }

            if (b.num_timing_points >= beatmap::max_timing_points) {
                die("Too many timing points for the internal buffer");
                return;
            }

            timing_point& tp = b.timing_points[b.num_timing_points];

            u16 not_inherited = 0;
            f64 time_tmp; // I'm rounding times to milliseconds.
                          // not sure if making them floats will matter for diff

            if (sscanf(tok,
                       "%lf,%lf,%" fi32 ",%" fi32 ",%" fi32 ",%" fi32 ",%" fu16,
                       &time_tmp, &tp.ms_per_beat,
                       &useless, &useless, &useless, &useless,
                       &not_inherited) == 7) {

                tp.time = (i64)time_tmp;
                tp.inherit = not_inherited == 0;
                goto parsed_timing_pt;
            }

            // older formats might not have inherit and the other info
            if (sscanf(tok, "%lf,%lf", &time_tmp, &tp.ms_per_beat) != 2)
            {
                tp.time = (i64)time_tmp;
                die("Invalid format for timing point");
                return;
            }

    parsed_timing_pt:
            b.num_timing_points++;
        }

        // ---

        profile(prid, "hit objects");

        if (!find_fwd(tok, "[HitObjects]")) {
            die("Could not find HitObjects");
            return;
        }

        // ---

        for (; tok; fwd())
        {
#if OPPAI_PROFILING
            const int prid = 2;
#endif

            profile(prid, "loop start");

            if (whitespace(tok)) {
                dbgputs("skipping whitespace line");
                continue;
            }

            if (b.num_objects >= beatmap::max_objects) {
                die("Too many hit objects for the internal buffer");
                return;
            }

            profile(prid, "object type");
            hit_object& ho = b.objects[b.num_objects];

            i32 type_num;

            // slider
            if (sscanf(tok, "%f,%f,%" fi64 ",%*" fi32 ",%*" fi32 ",%c",
                       &ho.pos.x, &ho.pos.y, &ho.time, &ho.slider.type) == 4 &&
                        ho.slider.type >= 'A' && ho.slider.type <= 'Z') {

                // the slider type check is for old maps that have trailing
                // commas on circles and sliders

                // x,y,time,type,hitSound,sliderType|curveX:curveY|...,repeat,
                //      pixelLength,edgeHitsound,edgeAddition,addition
                ho.type = obj::slider;
            }

            // circle, or spinner
            else if (sscanf(tok, "%f,%f,%" fi64 ",%" fi32 ",%" fi32 ",%" fi64,
                       &ho.pos.x, &ho.pos.y, &ho.time, &type_num, &useless,
                       &ho.end_time) == 6) {

                if (type_num & 8) {
                    // x,y,time,type,hitSound,endTime,addition
                    ho.type = obj::spinner;
                } else {
                    // x,y,time,type,hitSound,addition
                    ho.type = obj::circle;
                    ho.end_time = ho.time;
                }
            }

            // old circle
            else if (sscanf(tok, "%f,%f,%" fi64 ",%" fi32 ",%" fi32 "",
                &ho.pos.x, &ho.pos.y, &ho.time, &type_num, &useless) == 5) {

                ho.type = obj::circle;
                ho.end_time = ho.time;
            }

            else {
                die("Invalid hit object found");
                return;
            }

            b.num_objects++;
            dbgprintf("\n\nobject %zd\n", b.num_objects);

            // increase max combo and circle/slider count
            b.max_combo++; // slider ticks are calculated later
            switch (ho.type) {
                case obj::circle:
                    dbgputs("it's a circle!");
                    b.num_circles++;
                    break;

                case obj::slider:
                    dbgputs("it's a slider!");
                    b.num_sliders++;
                    break;

                case obj::spinner:
                    dbgputs("it's a spinner!");
                    b.num_spinners++;
                    break;

                case obj::invalid:
                    die("How did you get here????????");
                    return;
            }

            // slider points are separated by |
            if (!strstr(tok, "|")) {
                dbgputs("no slider points, we're done here");
                continue;
            }

            // not a slider yet slider points were found
            if (ho.type != obj::slider) {
                die("Invalid slider found");
                return;
            }

            profile(prid, "slider points");
            slider_data& sl = ho.slider;

            // gotta make a copy of the line to tokenize sliders without
            // affecting the current line-by-line tokenization
            std::string sline(tok);
            char* line = &sline[0];

            char* saveptr = 0;
            char* slider_tok = strtok_r(line, "|", &saveptr);
            slider_tok = strtok_r(0, "|", &saveptr); // skip first token

            // I don't use sliders anymore

            // TODO: prevent useless copying here and in other vector usages
            //sl.points.push_back(ho.pos);
            dbgputs("first slider point");

            for (; slider_tok; slider_tok = strtok_r(0, "|", &saveptr)) {
                //sl.points.push_back(v2f{});
                //v2f& pt = sl.points[sl.points.size() - 1];
                //v2f pt;

                // lastcurveX:lastcurveY,repeat,pixelLength,
                //      edgeHitsound,edgeAddition,addition
                if (sscanf(slider_tok,
                          "%*f:%*f,%" fu16 ",%lf",
                          /*&pt.x, &pt.y,*/ &sl.repetitions, &sl.length) == 2) {

                    dbgputs("last slider point");
                    // end of point list
                    break;
                }

                // curveX:curveY
                //else if (sscanf(slider_tok, "%f:%f", &pt.x, &pt.y) != 2) {
                //    die("Invalid slider found");
                //    return;
                //}

                //dbgprintf("slider point %zd\n", sl.points.size());
            }

            profile(prid, "timing calculations");
            // find which timing section the slider belongs to
            timing_point* tp = b.timing(ho.time);
            timing_point* parent = b.parent_timing(tp);
            if (err()) {
                return;
            }

            // calculate slider velocity multiplier for inherited sections
            f64 sv_multiplier = 1;
            if (tp->inherit && tp->ms_per_beat < 0) {
                sv_multiplier = (-100.0 / tp->ms_per_beat);
            }

            // calculate slider end time
            f64 px_per_beat = b.sv * 100.0 * sv_multiplier;
            f64 num_beats = (sl.length * sl.repetitions) / px_per_beat;
            i64 duration = (i64)std::ceil(num_beats * parent->ms_per_beat);
            ho.end_time = ho.time + duration;

            // sliders get 2 + ticks combo (head, tail and ticks)
            // each repetition adds an extra combo and an extra set of ticks

            // calculate the number of slider ticks for one repetition
            // ---
            // example: a 3.75 beats slider at 1x tick rate will go:
            // beat0 (head), beat1 (tick), beat2(tick), beat3(tick), beat3.75(tail)
            // so all we have to do is ceil the number of beats and subtract 1 to
            // take out the tail
            // ---
            // the -.01 is there to prevent ceil from ceiling whole values
            // like 1.0 to 2.0 randomly
            u16 ticks = (u16)std::ceil(
                    (num_beats - 0.1) / sl.repetitions * b.tick_rate);
            ticks--;

            ticks *= sl.repetitions; // multiply slider ticks by repetitions
            ticks += sl.repetitions + 1; // add heads and tails

            dbgprintf("%g beats x %" fu16 " = %" fu16 " combo\n",
                num_beats, sl.repetitions, ticks);

            b.max_combo += ticks - 1; // -1 because we already did ++ earlier
        }

        profile(prid, "");

        if (!disable_cache && !cachefd)
        {
            cachefd = fopen(cachefile, "wb");
            if (!cachefd) {
                perror("fopen");
            } else {
                b.write(cachefd);
                fclose(cachefd);
                dbgputs("beatmap written to cache");
            }
        }

        dbgputs("\nparsing done");
    }

    // get timing point at the given time
    timing_point* timing(i64 time) {
        for (i64 i = (i64)num_timing_points - 1; i >= 0; i--) {
            timing_point& cur = timing_points[i];

            if (cur.time <= time) {
                return &cur;
            }
        }

        return &timing_points[0];
    }

    // find parent of a inherited timing point
    timing_point* parent_timing(timing_point* t)
    {
        timing_point* res = 0;

        if (!t->inherit) {
            return t;
        }

        for (i64 i = (i64)num_timing_points - 1; i >= 0; i--)
        {
            timing_point& cur = timing_points[i];

            if (cur.time <= t->time && !cur.inherit)
            {
                if (!res || res->time > res->time) {
                    res = &cur;
                }
            }
        }

        if (!res) {
            die("Orphan timing section");
        }

        return res;
    }

    // apply map-modifying mods (such as EZ, HR, DT, HT)
    void apply_mods(u32 mods) {
        if ((mods & mods::map_changing) == 0) {
            return;
        }

        // playback speed
        f32 speed = 1;

        if ((mods & mods::dt) != 0 || (mods & mods::nc) != 0) {
            speed *= 1.5f;
        }

        if (mods & mods::ht) {
            speed *= 0.75f;
        }

        // od
        f32 od_ar_hp_multiplier = 1;

        if (mods & mods::hr) {
            od_ar_hp_multiplier *= 1.4f;
        }

        if (mods & mods::ez) {
            od_ar_hp_multiplier *= 0.5f;
        }

        od *= od_ar_hp_multiplier;
        f32 odms = od0_ms - std::ceil(od_ms_step * od);

        // ar
        ar *= od_ar_hp_multiplier;

        // hp
        hp *= od_ar_hp_multiplier;

        // convert AR into its milliseconds value
        f32 arms = ar <= 5
            ? (ar0_ms - ar_ms_step1 *  ar     )
            : (ar5_ms - ar_ms_step2 * (ar - 5));

        // cs
        f32 cs_multiplier = 1;

        if (mods & mods::hr) {
            cs_multiplier = 1.3f;
        }

        if (mods & mods::ez) {
            cs_multiplier = 0.5f;
        }

        // stats must be capped to 0-10 before HT/DT which bring them to a range
        // of -4.42 to 11.08 for OD and -5 to 11 for AR
        odms = std::min(od0_ms, std::max(od10_ms, odms));
        arms = std::min(ar0_ms, std::max(ar10_ms, arms));

        // apply speed-changing mods
        odms /= speed;
        arms /= speed;

        // convert OD and AR back into their stat form
        //od = (-(odms - od0_ms)) / od_ms_step;
        od = (od0_ms - odms) / od_ms_step;
        ar = ar <= 5.0f
            //? (      (-(arms - ar0_ms)) / ar_ms_step1)
            //: (5.0 + (-(arms - ar5_ms)) / ar_ms_step2);
            ? (       (ar0_ms - arms) / ar_ms_step1)
            : (5.0f + (ar5_ms - arms) / ar_ms_step2);

        cs *= cs_multiplier;
        cs = std::max(0.0f, std::min(10.0f, cs));

        if ((mods & mods::speed_changing) == 0) {
            // not speed-modifying
            return;
        }

        // apply speed-changing mods

        for (size_t i = 0; i < num_timing_points; i++) {
            timing_point&tp = timing_points[i];
            tp.time = (i64)((f64)tp.time / speed);
            if (!tp.inherit) {
                tp.ms_per_beat /= speed;
            }
        }

        for (size_t i = 0; i < num_objects; i++) {
            hit_object& o = objects[i];
            o.time = (i64)((f64)o.time / speed);
            o.end_time = (i64)((f64)o.end_time / speed);
        }
    }
};

