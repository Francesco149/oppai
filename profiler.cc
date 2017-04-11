#if OPPAI_PROFILING
#include <time.h>
#include <map>
#include <string>

// profiling is linux-only for now since it's mainly for myself
f64 time_now()
{
    struct timespec t;

    memset(&t, 0, sizeof(struct timespec));

    if (clock_gettime(CLOCK_MONOTONIC, &t) < 0) {
        perror(0);
        exit(1);
    }

    return t.tv_sec + t.tv_nsec * (f64)1e-9;
}

#define MAX_PROFILERS 3

char const* profile_last_name[MAX_PROFILERS];
f64 profile_last[MAX_PROFILERS];

struct iterations
{
    u32 n;
    f64 sum;

    iterations() : n(0), sum(0) {}
};

// TODO: don't use map
std::map<std::string, iterations> profile_iterations[MAX_PROFILERS];

void profile_init() {
    memset(profile_last_name, 0, sizeof(profile_last_name));
    memset(profile_last, 0, sizeof(profile_last));
}

void profile(int i, char const* name)
{
    if (i > MAX_PROFILERS - 1) {
        fprintf(stderr, "bruh fix your profilers\n");
        exit(1);
    }

    f64 now = time_now();

    if (profile_last_name[i])
    {
        iterations& it = profile_iterations[i][profile_last_name[i]];
        ++it.n;
        it.sum += now - profile_last[i];
    }

    profile_last[i] = now;
    profile_last_name[i] = name;
}

void profile_end()
{
    for (int i = 0; i < MAX_PROFILERS; ++i)
    {
        f64 total_time = 0;

        for (std::map<std::string, iterations>::iterator pair =
                profile_iterations[i].begin();
             pair != profile_iterations[i].end();
             ++pair)
        {
            total_time += pair->second.sum;
        }

        for (std::map<std::string, iterations>::iterator pair =
                profile_iterations[i].begin();
             pair != profile_iterations[i].end();
             ++pair)
        {
            for (int j = 0; j < i; ++j) {
                fprintf(stderr, "  ");
            }

            fprintf(
                stderr,
                "PROFILER%d|%s: %gs (%g%%)\n", i,
                pair->first.c_str(), pair->second.sum / pair->second.n,
                pair->second.sum / total_time * 100.0
            );
        }
    }
}
#else
#define profile_init()
#define profile(a, b)
#define profile_end()
#endif
