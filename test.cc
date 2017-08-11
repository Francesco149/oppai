#define OPPAI_STDINT 1
#define OPPAI_LIB 1
#include "main.cc"
#include "test_suite.cc" /* defines suite */

#define BUFSIZE 8000000
#define ERROR_MARGIN 0.02 /* pp can be off by +- 2% */
/*
    margin is actually
    - 3x for < 100pp
    - 2x for 100-200pp
    - 1.5x for 200-300pp
*/

#include <curl/curl.h>

static size_t wrchunk(void* p, size_t cb, size_t nmemb, void* fd) {
    return fwrite(p, cb, nmemb, (FILE*)fd);
}

static void check_err(oppai_ctx* ctx)
{
    if (oppai_err(ctx)) {
        fprintf(stderr, "%s\n", oppai_err(ctx));
        exit(1);
    }
}

static void print_score(struct score* s)
{
    char mods_str_buf[20];
    char* mods_str = mods_str_buf;

    strcpy(mods_str, "nomod");

#   define m(mod) \
    if (s->mods & mods::mod) { \
        mods_str += sprintf(mods_str, #mod); \
    } \

    m(hr) m(nc) m(ht) m(so) m(nf) m(ez) m(dt) m(fl) m(hd)
#   undef m

    fprintf(
        stderr,
        "%" fu32 " +%s "
        "%" fu16 "x "
        "%" fu16 "x300 "
        "%" fu16 "x100 "
        "%" fu16 "x50 "
        "%" fu16 "xmiss "
        "%g pp\n",
        s->id, mods_str_buf,
        s->max_combo, s->n300, s->n100, s->n50, s->nmiss, s->pp
    );
}

int main(int argc, char* argv[])
{
    CURL* curl = 0;

    char* buf;
    char fname_buf[4096];
    char url_buf[128];
    char* fname = fname_buf;
    char* url = url_buf;

    uint32_t i;
    uint32_t n = (uint32_t)(sizeof(suite) / sizeof(suite[0]));

    buf = (char*)malloc(BUFSIZE);
    mkdir("test_suite");
    fname += sprintf(fname, "test_suite/");

    for (i = 0; i < n; ++i)
    {
        struct score* s = &suite[i];
        double aim = 0, speed = 0;
        double margin;

        print_score(s);
        sprintf(fname, "%" fu32 ".osu", s->id);

trycalc:
        oppai_ctx ctx;
        beatmap b(&ctx);
        beatmap::parse(fname_buf, b, buf, BUFSIZE, 1);

        if (oppai_err(&ctx))
        {
            /* TODO: properly error check&log curl
                     also pull this out into a function maybe */

            CURLcode res;
            FILE* f;

            fprintf(stderr, "%s\n", oppai_err(&ctx));
            ctx.last_err = 0;

            if (!curl)
            {
                fprintf(stderr, "initializing curl\n");
                curl_global_init(CURL_GLOBAL_ALL);

                curl = curl_easy_init();
                if (!curl) {
                    fprintf(stderr, "curl_easy_init failed\n");
                    exit(1);
                }

                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
                curl_easy_setopt(
                    curl, CURLOPT_WRITEFUNCTION, wrchunk
                );

                url += sprintf(url, "http://osu.ppy.sh/osu/");
            }

            sprintf(url, "%" fu32, s->id);
            curl_easy_setopt(curl, CURLOPT_URL, url_buf);

            fprintf(stderr, "downloading %s\n", url_buf);
            f = fopen(fname_buf, "wb");
            if (!f) {
                perror("fopen");
                exit(1);
            }

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
            res = curl_easy_perform(curl);

            if (res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform failed");
                fclose(f);
                unlink(fname_buf);
                goto trycalc;
            }

            fclose(f);
            goto trycalc;
        }

        b.apply_mods(s->mods);

        d_calc_ctx dctx(&ctx);
        d_calc(&dctx, b, &aim, &speed);
        check_err(&ctx);

        pp_calc_result res =
            pp_calc(
                &ctx, aim, speed, b,
                s->mods, s->max_combo, s->nmiss,
                s->n300, s->n100, s->n50
            );

        check_err(&ctx);

        margin = s->pp * ERROR_MARGIN;
        if (s->pp < 100) {
            margin *= 3;
        }
        else if (s->pp < 200) {
            margin *= 2;
        }
        else if (s->pp < 300) {
            margin *= 1.5;
        }

        if (fabs(res.pp - s->pp) >= margin)
        {
            fprintf(
                stderr,
                "failed test: got %g pp, expected %g\n",
                res.pp, s->pp
            );

            exit(1);
        }
    }

    return 0;
}

