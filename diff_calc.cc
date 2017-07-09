#include <functional> // std::greater
#include <vector>

#define macro_round(x) std::floor((x) + 0.5)

// based on tom94's osu!tp aimod
// TODO: rewrite this to be less object oriented

// how much strains decay per interval (if the previous interval's peak
// strains after applying decay are still higher than the current one's,
// they will be used as the peak strains).
const f64 decay_base[] = { 0.3, 0.15 };

// almost the normalized circle diameter (104px)
const f64 almost_diameter = 90;

// arbitrary tresholds to determine when a stream is spaced enough that is
// becomes hard to alternate.
const f64 stream_spacing = 110;
const f64 single_spacing = 125;

// used to keep speed and aim balanced between eachother
const f64 weight_scaling[] = { 1400, 26.25 };

// non-normalized diameter where the circlesize buff starts
const f32 circlesize_buff_treshold = 30;

namespace diff
{
    const u8 speed = 0,
             aim = 1;
}

// diffcalc hit object
struct d_obj
{
    oppai_ctx* ctx;
    hit_object* ho;

    f64 strains[2];

    // start/end positions normalized on radius
    v2f norm_start;
    v2f norm_end;

    // initialized after claulcating strains
    bool is_single;

    d_obj()
    {
        // strains start at 1
        strains[0] = 1;
        strains[1] = 1;
    }

    void init(oppai_ctx* ctx, hit_object* base_object, f32 radius)
    {
        this->ctx = ctx;
        this->ho = base_object;

        // positions are normalized on circle radius so that we can calc as
        // if everything was the same circlesize
        f32 scaling_factor = 52.0f / radius;

        // cs buff (credits to osuElements, I have confirmed that this is
        // indeed accurate)
        if (radius < circlesize_buff_treshold)
        {
            scaling_factor *= 1.f +
                std::min((circlesize_buff_treshold - radius), 5.f) / 50.f;
        }

        norm_start = ho->pos * scaling_factor;

        norm_end = norm_start;
        // ignoring slider lengths doesn't seem to affect star rating too
        // much and speeds up the calculation exponentially
    }

    void calculate_strains(d_obj& prev)
    {
        calculate_strain(prev, diff::speed);
        if (oppai_err(ctx)) {
            return;
        }

        calculate_strain(prev, diff::aim);
    }

    void calculate_strain(d_obj& prev, u8 dtype)
    {
        f64 res = 0;
        i32 time_elapsed = ho->time - prev.ho->time;
        f64 decay = pow(decay_base[dtype], time_elapsed / 1000.0);
        f64 scaling = weight_scaling[dtype];

        switch (ho->type)
        {
            case obj::slider: // we don't use sliders in this implementation
            case obj::circle:
                res = spacing_weight(distance(prev), dtype) * scaling;
                break;

            case obj::spinner:
                break;

            case obj::invalid:
                die(ctx, "Found invalid hit object");
                return;
        }

        res /= std::max(time_elapsed, (i32)50);
        strains[dtype] = prev.strains[dtype] * decay + res;
    }

    f64 spacing_weight(f64 distance, u8 diff_type)
    {
        switch (diff_type)
        {
            case diff::speed:
                if (distance > single_spacing) {
                    is_single = true;
                    return 2.5;
                }
                else if (distance > stream_spacing) {
                    return 1.6 + 0.9 *
                        (distance - stream_spacing) /
                        (single_spacing - stream_spacing);
                }
                else if (distance > almost_diameter) {
                    return 1.2 + 0.4 * (distance - almost_diameter)
                        / (stream_spacing - almost_diameter);
                }
                else if (distance > almost_diameter / 2.0) {
                    return 0.95 + 0.25 *
                        (distance - almost_diameter / 2.0) /
                        (almost_diameter / 2.0);
                }
                return 0.95;

            case diff::aim:
                return pow(distance, 0.99);

            default:
                return 0.0;
        }
    }

    f64 distance(d_obj& prev) {
        return (norm_start - prev.norm_end).len();
    }
};

const f64 star_scaling_factor = 0.0675;
const f64 extreme_scaling_factor = 0.5;
const f32 playfield_width = 512.f; // in osu!pixels

// strains are calculated by analyzing the map in chunks and then taking the
// peak strains in each chunk.
// this is the length of a strain interval in milliseconds.
const i32 strain_step = 400;

// max strains are weighted from highest to lowest, and this is how much the
// weight decays.
const f64 decay_weight = 0.9;

struct d_calc_ctx
{
    oppai_ctx* octx;
    d_obj objects[beatmap::max_objects];
    size_t num_objects;

    d_calc_ctx(oppai_ctx* octx) : octx(octx) {}
};

internalfn
f64 calculate_difficulty(d_calc_ctx* ctx, u8 type)
{
    std::vector<f64> highest_strains;
    i32 interval_end = strain_step;
    f64 max_strain = 0.0;

    d_obj* prev = 0;

    for (size_t i = 0; i < ctx->num_objects; i++)
    {
        d_obj& o = ctx->objects[i];

        // make previous peak strain decay until the current object
        while (o.ho->time > interval_end)
        {
            highest_strains.push_back(max_strain);

            if (!prev) {
                max_strain = 0.0;
            } else {
                f64 decay = pow(decay_base[type],
                    (interval_end - prev->ho->time) / 1000.0);
                max_strain = prev->strains[type] * decay;
            }

            interval_end += strain_step;
        }

        // calculate max strain for this interval
        max_strain = std::max(max_strain, o.strains[type]);
        prev = &o;
    }

    f64 difficulty = 0;
    f64 weight = 1.0;

    // sort strains from greatest to lowest
    std::sort(highest_strains.begin(), highest_strains.end(),
        std::greater<f64>());

    // weigh the top strains
    for (std::vector<f64>::iterator it = highest_strains.begin();
         it != highest_strains.end();
         ++it)
    {
        difficulty += weight * *it;
        weight *= decay_weight;
    }

    return difficulty;
}

// calculates overall aim, speed and stars for a map.
//
// aim, speed: pointers to the variables where
//             aim and speed stars will be stored.
//
// rhythm_awkwardness: optional pointer that will store the experimental
//                     rhythm awkwardness stat.
//
// nsingles: optional pointer that will store the number of aim singletaps.
//           aim singletaps are singletaps as seen by the difficulty calculator,
//           computed based on spacing thresholds.
//
// nsingles_timing: optional pointer that will store the number of timing
//                  singletaps (1/2 or slower notes).
//
// nsingles_threshold: optional pointer that will store the number of notes
//                     equal or slower than 1/2 notes at singletap_threshold ms
//
// returns star rating

OPPAIAPI
f64 d_calc(
    d_calc_ctx* ctx,
    beatmap& b, f64* aim, f64* speed,
    f64* rhythm_awkwardness = 0,
    u16* nsingles = 0,
    u16* nsingles_timing = 0,
    u16* nsingles_threshold = 0,
    i32 singletap_threshold = 125)
{
    dbgputs("\ndiff calc");

    if (b.mode != 0) {
        die(ctx->octx, "This gamemode is not supported");
        return 0;
    }

    f32 circle_radius = (playfield_width / 16.f) * (1.f - 0.7f *
        ((f32)b.cs - 5.f) / 5.f);

    dbgprintf("circle radius: %g\n", circle_radius);

    ctx->num_objects = b.num_objects;
    dbgputs("initializing objects");

    for (size_t i = 0; i < b.num_objects; i++)
    {
        ctx->objects[i].init(ctx->octx, &b.objects[i], circle_radius);
        if (oppai_err(ctx->octx)) {
            return 0;
        }
    }

    // TODO: don't use vector
    std::vector<i32> intervals;

    d_obj* prev = &ctx->objects[0];
    for (size_t i = 1; i < b.num_objects; i++)
    {
        d_obj& o = ctx->objects[i];

        o.calculate_strains(*prev);
        if (oppai_err(ctx->octx)) {
            return 0;
        }

        dbgprintf("%" fi32 ": type %" fi32 ", strains %g %g, "
                "norm pos %s-%s, pos %s\n",
                o.ho->time, (int)o.ho->type, o.strains[0], o.strains[1],
                o.norm_start.str(), o.norm_end.str(), o.ho->pos.str());

        i32 interval = o.ho->time - prev->ho->time;
        intervals.push_back(interval);

        if (nsingles && o.is_single) {
            ++*nsingles;
        }

        i32 one_half_threshold = (i32)(b.timing(o.ho->time)->ms_per_beat / 2);

        if (nsingles_timing && interval >= one_half_threshold) {
            ++*nsingles_timing;
        }

        if (nsingles_threshold && interval >= singletap_threshold) {
            ++*nsingles_threshold;
        }

        prev = &o;
    }

    std::vector<i32> group;

    u32 noffsets = 0;

    if (!rhythm_awkwardness) {
        goto skip_awkwardness;
    }

    *rhythm_awkwardness = 0;
    for (size_t i = 0; i < intervals.size(); ++i)
    {
        // TODO: actually compute break time length for the map's AR
        bool isbreak = intervals[i] >= 1200;

        if (!isbreak) {
            group.push_back(intervals[i]);
        }

        if (isbreak || group.size() >= 5 || i == intervals.size() - 1)
        {
            for (size_t j = 0; j < group.size(); ++j)
            {
                for (size_t k = 1; k < group.size(); ++k)
                {
                    if (k == j) {
                        continue;
                    }

                    f64 ratio = group[j] > group[k] ?
                        (f64)group[j] / (f64)group[k] :
                        (f64)group[k] / (f64)group[j];

                    f64 closest_pot = pow(2, macro_round(log(ratio)/log(2.0)));

                    f64 offset = std::abs(closest_pot - ratio);
                    offset /= closest_pot;

                    *rhythm_awkwardness += offset * offset;

                    ++noffsets;
                }
            }

            group.clear();
        }
    }

    *rhythm_awkwardness /= noffsets;
    *rhythm_awkwardness *= 82;

skip_awkwardness:
    *aim   = calculate_difficulty(ctx, diff::aim);
    *speed = calculate_difficulty(ctx, diff::speed);

    *aim   = sqrt(*aim)   * star_scaling_factor;
    *speed = sqrt(*speed) * star_scaling_factor;

    f64 stars = *aim + *speed +
        std::abs(*speed - *aim) * extreme_scaling_factor;

    return stars;
}

