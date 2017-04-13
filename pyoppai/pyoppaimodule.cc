#include "../main.cc"
#include <Python.h>

#ifdef OPPAI_MODULE_DEBUG
#define pydbgprintf(fmt, ...)           \
    fprintf(stderr, "DEBUG|pyoppai|");  \
    fprintf(stderr, fmt, __VA_ARGS__)
#else
#define pydbgprintf(fmt, ...)
#endif

#define pydbgputs(msg) pydbgprintf("%s\n", msg)

// TODO: don't use new/malloc and pre-allocate a block of memory to use as a
//       stack/custom allocator instead

extern "C"
{
    // -------------------------------------------------------------------------

#define destructor(tname)                                                  \
    internalfn                                                             \
    void tname##_free(PyObject* capsule)                                   \
    {                                                                      \
        tname* p = (tname*)                                                \
            PyCapsule_GetPointer(capsule, "__CPP_API_" #tname "__");       \
                                                                           \
        if (!p) {                                                          \
            pydbgputs("WARNING|destructor called on null " #tname);        \
            return;                                                        \
        }                                                                  \
                                                                           \
        pydbgprintf("deallocating " #tname " %p\n", p);                    \
                                                                           \
        delete p;                                                          \
    }

#define encapsulate(tname, p)       \
    PyCapsule_New(                  \
        (void*)p,                   \
        "__CPP_API_" #tname "__",   \
        tname##_free                \
    );

#define unencapsulate(tname, p) \
    (tname*)PyCapsule_GetPointer(p, "__CPP_API_" #tname "__")

    // -------------------------------------------------------------------------

    destructor(oppai_ctx)

    internalfn
    PyObject* pyoppai_new_ctx(PyObject* self, PyObject* args)
    {
        oppai_ctx* ctx = new oppai_ctx;
        pydbgprintf("allocated oppai_ctx %p\n", ctx);
        return encapsulate(oppai_ctx, ctx);
    }

    // -------------------------------------------------------------------------

    internalfn
    PyObject* pyoppai_err(PyObject* self, PyObject* args)
    {
        PyObject* capsule;

        if (!PyArg_ParseTuple(args, "O", &capsule)) {
            return 0;
        }

        oppai_ctx* p = unencapsulate(oppai_ctx, capsule);

        char const* str = "";

        if (oppai_err(p)) {
            str = oppai_err(p);
        }

        return Py_BuildValue("s", str);
    }

    // -------------------------------------------------------------------------

    destructor(beatmap)

    internalfn
    PyObject* pyoppai_new_beatmap(PyObject* self, PyObject* args)
    {
        PyObject* capsule;

        if (!PyArg_ParseTuple(args, "O", &capsule)) {
            return 0;
        }

        oppai_ctx* ctx = unencapsulate(oppai_ctx, capsule);
        beatmap* b = new beatmap(ctx);
        pydbgprintf("allocated beatmap %p\n", b);

        return encapsulate(beatmap, b);
    }

    // -------------------------------------------------------------------------

    internalfn
    void buffer_free(PyObject* capsule)
    {
        char* p = (char*)
            PyCapsule_GetPointer(capsule, "__CPP_API_oppai_buffer__");

        if (!p) {
            pydbgputs("WANRING|destructor called on empty buffer");
            return;
        }

        pydbgprintf("deallocating buffer %p\n", p);

        free(p);
    }

    internalfn
    PyObject* pyoppai_new_buffer(PyObject* self, PyObject* args)
    {
        u32 len;

        if (!PyArg_ParseTuple(args, "I", &len)) {
            return 0;
        }

        char* buf = (char*)malloc(len);
        pydbgprintf("allocated buffer %p\n", buf);

        return PyCapsule_New(
            (void*)buf,
            "__CPP_API_oppai_buffer__",
            buffer_free
        );
    }

    // -------------------------------------------------------------------------

    internalfn
    PyObject* pyoppai_apply_mods(PyObject* self, PyObject* args)
    {
        PyObject* capsule;
        u32 mods_mask;

        if (!PyArg_ParseTuple(args, "OI", &capsule, &mods_mask)) {
            return 0;
        }

        beatmap* p = unencapsulate(beatmap, capsule);
        p->apply_mods((u32)mods_mask);

        Py_RETURN_NONE;
    }

    // -------------------------------------------------------------------------

    internalfn
    PyObject* pyoppai_parse(PyObject* self, PyObject* args)
    {
        char const* filepath;
        PyObject* b_capsule;
        PyObject* buf_capsule;
        u32 bufsize;
        int disable_cache = 0;
        char const* custom_cache_path;

        int pres =
            PyArg_ParseTuple(
                args, "sOOI|is",
                &filepath, &b_capsule, &buf_capsule, &bufsize,
                &disable_cache, &custom_cache_path
            );

        if (!pres) {
            return 0;
        }

        beatmap* b = unencapsulate(beatmap, b_capsule);

        char* buf = (char*)
            PyCapsule_GetPointer(buf_capsule, "__CPP_API_oppai_buffer__");

        pydbgprintf(
            "beatmap::parse(\"%s\", %p, %p, %" fu32 ", %d, \"%s\")\n",
            filepath, b, buf, bufsize, disable_cache, custom_cache_path
        );

        beatmap::parse(
            filepath,
            *b,
            buf,
            bufsize,
            disable_cache != 0,
            custom_cache_path
        );

        Py_RETURN_NONE;
    }

    // -------------------------------------------------------------------------

    destructor(d_calc_ctx)

    internalfn
    PyObject* pyoppai_new_d_calc_ctx(PyObject* self, PyObject* args)
    {
        PyObject* capsule;

        if (!PyArg_ParseTuple(args, "O", &capsule)) {
            return 0;
        }

        oppai_ctx* ctx = unencapsulate(oppai_ctx, capsule);
        d_calc_ctx* dctx = new d_calc_ctx(ctx);
        pydbgprintf("allocated d_calc_ctx %p\n", dctx);

        return encapsulate(d_calc_ctx, dctx);
    }

    // -------------------------------------------------------------------------

    internalfn
    PyObject* pyoppai_d_calc(PyObject* self, PyObject* args)
    {
        PyObject* ctx_capsule;
        PyObject* b_capsule;
        int with_awkwardness = 0;
        int with_aim_singles = 0;
        int with_timing_singles = 0;
        int with_threshold_singles = 0;
        i32 singletap_threshold = 240;

        int ptres =
            PyArg_ParseTuple(
                args,
                "OO|iiiii",
                &ctx_capsule,
                &b_capsule,
                &with_awkwardness,
                &with_aim_singles,
                &with_timing_singles,
                &with_threshold_singles,
                &singletap_threshold
            );

        if (!ptres) {
            return 0;
        }

        d_calc_ctx* ctx = unencapsulate(d_calc_ctx, ctx_capsule);
        beatmap* b = unencapsulate(beatmap, b_capsule);

        f64 aim = 0, speed = 0, rhythm_awkwardness = 0;
        u16 nsingles = 0, nsingles_timing = 0, nsingles_threshold = 0;

        f64 stars =
            d_calc(
                ctx, *b, &aim, &speed,
                with_awkwardness ? &rhythm_awkwardness : 0,
                with_aim_singles ? &nsingles : 0,
                with_timing_singles ? &nsingles_timing : 0,
                with_threshold_singles ? &nsingles_threshold : 0
            );

        pydbgprintf(
            "d_calc(%p, %p, %p, %p, %p, %p, %p, %p)\n",
            ctx, b, &aim, &speed,
            with_awkwardness ? &rhythm_awkwardness : 0,
            with_aim_singles ? &nsingles : 0,
            with_timing_singles ? &nsingles_timing : 0,
            with_threshold_singles ? &nsingles_threshold : 0
        );

        return
            Py_BuildValue(
                "(ddddHHH)",
                stars, aim, speed, rhythm_awkwardness,
                nsingles, nsingles_timing, nsingles_threshold
            );
    }

    // -------------------------------------------------------------------------

    internalfn
    PyObject* pyoppai_pp_calc(PyObject* self, PyObject* args)
    {
        PyObject* ctx_capsule;
        f64 aim, speed;
        PyObject* b_capsule;
        u32 used_mods = mods::nomod;
        u16 combo = 0xFFFF, misses = 0, c300 = 0xFFFF, c100 = 0, c50 = 0;
        u32 score_version = 1;

        int ptres =
            PyArg_ParseTuple(
                args,
                "OddO|IHHHHHI",
                &ctx_capsule,
                &aim, &speed,
                &b_capsule,
                &used_mods,
                &combo, &misses, &c300, &c100, &c50,
                &score_version
            );

        if (!ptres) {
            return 0;
        }

        oppai_ctx* ctx = unencapsulate(oppai_ctx, ctx_capsule);
        beatmap* b = unencapsulate(beatmap, b_capsule);

        pp_calc_result res =
            pp_calc(
                ctx,
                aim, speed,
                *b,
                used_mods,
                combo, misses, c300, c100, c50,
                score_version
            );

        pydbgprintf(
            "pp_calc(%p, %.17g, %.17g, %p, %08X, "
            "%" fu16 ", %" fu16 ", %" fu16 ", %" fu16 ","
            " %" fu16 ", %" fu32 ")\n",
            ctx,
            aim, speed,
            b,
            used_mods,
            combo, misses, c300, c100, c50,
            score_version
        );

        return
            Py_BuildValue(
                "(ddddd)",
                res.acc_percent, res.pp, res.aim_pp, res.speed_pp, res.acc_pp
            );
    }

    // -------------------------------------------------------------------------

    internalfn
    PyObject* pyoppai_pp_calc_acc(PyObject* self, PyObject* args)
    {
        PyObject* ctx_capsule;
        f64 aim, speed;
        PyObject* b_capsule;
        f64 acc_percent = 100.0;
        u32 used_mods = mods::nomod;
        u16 combo = 0xFFFF, misses = 0;
        u32 score_version = 1;

        int ptres =
            PyArg_ParseTuple(
                args,
                "OddO|dIHHI",
                &ctx_capsule,
                &aim, &speed,
                &b_capsule,
                &acc_percent,
                &used_mods,
                &combo, &misses,
                &score_version
            );

        if (!ptres) {
            return 0;
        }

        oppai_ctx* ctx = unencapsulate(oppai_ctx, ctx_capsule);
        beatmap* b = unencapsulate(beatmap, b_capsule);

        pp_calc_result res =
            pp_calc_acc(
                ctx,
                aim, speed,
                *b,
                acc_percent,
                used_mods,
                combo, misses,
                score_version
            );

        pydbgprintf(
            "pp_calc_acc(%p, %.17g, %.17g, %p, %.17g"
            "%" fu16 ", %" fu16 ", %" fu32 ")\n",
            ctx,
            aim, speed,
            b,
            acc_percent,
            used_mods,
            combo, misses,
            score_version
        );

        return
            Py_BuildValue(
                "(ddddd)",
                res.acc_percent, res.pp, res.aim_pp, res.speed_pp, res.acc_pp
            );
    }

    // -------------------------------------------------------------------------

#define getter(tname, name, return_fmt, ...)                            \
    internalfn                                                          \
    PyObject* pyoppai_##name (PyObject* self, PyObject* args)           \
    {                                                                   \
        PyObject* capsule;                                              \
                                                                        \
        if (!PyArg_ParseTuple(args, "O", &capsule)) {                   \
            return 0;                                                   \
        }                                                               \
                                                                        \
        tname* p = unencapsulate(tname, capsule);                       \
                                                                        \
        return Py_BuildValue(return_fmt, __VA_ARGS__);                  \
    }

#define sgetter(tname, name, fmt) getter(tname, name, fmt, p->name)

    getter(beatmap, stats, "(ffff)", p->cs, p->od, p->ar, p->hp)

    sgetter(beatmap, mode,         "b")
    sgetter(beatmap, num_circles,  "H")
    sgetter(beatmap, num_sliders,  "H")
    sgetter(beatmap, num_spinners, "H")
    sgetter(beatmap, max_combo,    "H")
    sgetter(beatmap, num_objects,  "H")
    sgetter(beatmap, title,        "s")
    sgetter(beatmap, artist,       "s")
    sgetter(beatmap, creator,      "s")
    sgetter(beatmap, version,      "s")

    // -------------------------------------------------------------------------

#define setter(tname, name, vtype, vtype_specifier)                         \
    internalfn                                                              \
    PyObject* pyoppai_set_##name (PyObject* self, PyObject* args)           \
    {                                                                       \
        PyObject* capsule;                                                  \
        vtype v;                                                            \
                                                                            \
        if (!PyArg_ParseTuple(args, "O" vtype_specifier, &capsule, &v)) {   \
            return 0;                                                       \
        }                                                                   \
                                                                            \
        tname* p = unencapsulate(tname, capsule);                           \
        p->name = v;                                                        \
                                                                            \
        Py_RETURN_NONE;                                                     \
    }

#define fsetter(tname, name) setter(tname, name, f32, "f")

    fsetter(beatmap, cs)
    fsetter(beatmap, od)
    fsetter(beatmap, ar)

    // -------------------------------------------------------------------------

#define m(name, desc) { #name, pyoppai_##name, METH_VARARGS, desc }

    globvar PyMethodDef pyoppai_methods[] =
    {
        m(new_ctx,        "Create a new oppai context"),
        m(err,            "Returns last error description (empty str if none)"),
        m(new_beatmap,    "Create an empty beatmap object"),
        m(new_buffer,     "Allocate a buffer of n bytes (used by the parser)"),
        m(parse,          "Parses a .osu file into a beatmap object"),
        m(apply_mods,     "Applies mods to a beatmap"),
        m(new_d_calc_ctx, "Create a new difficulty calculation context"),
        m(d_calc,         "Calculates difficulty (star rating) for a beatmap"),
        m(pp_calc,        "Calculates ppv2 for a beatmap"),
        m(pp_calc_acc,    "Same as pp_calc but acc is provided in percent"),

#define g(tname, name, lname) m(name, "Gets a " #tname "'s " lname)

        g(beatmap, stats,        "stats (CS, OD, AR, HP)"),
        g(beatmap, mode,         "gamemode"),
        g(beatmap, num_circles,  "number of circles"),
        g(beatmap, num_sliders,  "number of sliders"),
        g(beatmap, num_spinners, "number of spinners"),
        g(beatmap, max_combo,    "maximum combo"),
        g(beatmap, num_objects,  "num. of objects (circles + sliders + spin)"),
        g(beatmap, title,        "title"),
        g(beatmap, artist,       "artist"),
        g(beatmap, creator,      "creator"),
        g(beatmap, version,      "version (difficulty name)"),

#undef g

#define s(tname, name, lname) \
    m(set_##name, "Sets a " #tname "'s " lname)

        s(beatmap, ar, "base approach rate"),
        s(beatmap, od, "base overall difficulty"),
        s(beatmap, cs, "base circle size"),

#undef s

        { 0, 0, 0, 0 }
    };

#undef m

    // -------------------------------------------------------------------------

    PyMODINIT_FUNC
    initpyoppai()
    {
        PyObject* m = Py_InitModule("pyoppai", pyoppai_methods);

#define mod(name) PyModule_AddIntConstant(m, #name, mods::name)
        mod(nomod);
        mod(nf);
        mod(ez);
        mod(hd);
        mod(hr);
        mod(dt);
        mod(hr);
        mod(dt);
        mod(ht);
        mod(nc);
        mod(fl);
        mod(so);
#undef mod
    }

    // -------------------------------------------------------------------------
}

int main(int argc, char* argv[])
{
    Py_SetProgramName(argv[0]);
    Py_Initialize();
    initpyoppai();

    return 0;
}
