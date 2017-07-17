#include "main.cc"

#ifdef OPPAI_MODULE_DEBUG
#define ndbgprintf(fmt, ...)           \
    fprintf(stderr, "DEBUG|node-oppai|");  \
    fprintf(stderr, fmt, __VA_ARGS__)
#else
#define ndbgprintf(fmt, ...)
#endif

#define ndbgputs(msg) ndbgprintf("%s\n", msg)

// I have no idea what I'm doing, this is mostly modified examples from node's
// documentation. v8's over-usage of templates makes me wanna die.

#include <node.h>
#include <node_object_wrap.h>

namespace oppai
{
    using v8::Isolate;
    using v8::Local;
    using v8::FunctionTemplate;
    using v8::String;
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::Function;
    using v8::Object;
    using v8::Context;
    using v8::Persistent;
    using v8::Number;
    using v8::Uint32;

    // -------------------------------------------------------------------------

    class Ctx : public node::ObjectWrap
    {
    public:
        oppai_ctx ctx;

		/**
         * **`oppai.Ctx`**
         *
         * Creates a pp calculation context for the current thread.
         * Each instance should not be used concurrently.
         *
         * @class Ctx
         */
        static void Init(Local<Object> exports)
        {
            Isolate* isolate = exports->GetIsolate();
            Local<FunctionTemplate> t = FunctionTemplate::New(isolate, New);

            t->SetClassName(String::NewFromUtf8(isolate, "Ctx"));
            t->InstanceTemplate()->SetInternalFieldCount(1);

            NODE_SET_PROTOTYPE_METHOD(t, "err", Err);
            NODE_SET_PROTOTYPE_METHOD(t, "ppCalc", PpCalc);
            NODE_SET_PROTOTYPE_METHOD(t, "ppCalcAcc", PpCalcAcc);

            constructor.Reset(isolate, t->GetFunction());
            exports->Set(String::NewFromUtf8(isolate, "Ctx"), t->GetFunction());
        }

        static void NewInstance(
            const FunctionCallbackInfo<Value>& args,
            Isolate* isolate)
        {
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Context> context = isolate->GetCurrentContext();
            Local<Object> instance =
                cons->NewInstance(context, 0, 0).ToLocalChecked();

            args.GetReturnValue().Set(instance);
        }

        static void NewInstance(const FunctionCallbackInfo<Value>& args)
        {
            Isolate* isolate = args.GetIsolate();
            NewInstance(args, isolate);
        }

    private:
        static Persistent<Function> constructor;

        explicit Ctx() {
            ndbgprintf("allocated oppai_ctx %p\n", &ctx);
        }

        ~Ctx() {
            ndbgprintf("deallocating oppai_ctx %p\n", &ctx);
        }

        static void New(const FunctionCallbackInfo<Value>& args)
        {
            Isolate* isolate = args.GetIsolate();

            if (args.IsConstructCall())
            {
                Ctx* c = new Ctx;
                c->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            }
            else {
                NewInstance(args, isolate);
            }
        }

		/**
         * Gets a description of the last error that occurred during oppai
         * calls. This should be checked after 'Beatmap.parse', 'Ctx.ppCalc',
         * 'Ctx.ppCalcAcc' and 'DiffCalcCtx.diffCalc'.
         * @name err
         * @memberof Ctx
         * @static
         * @returns {string} description of the last error or undefined if none
         * @example
         * var ctx = oppai.Ctx();
         * var b = oppai.Beatmap(ctx);
         * var buf = oppai.Buffer(2000000);
         * b.parse("some_file.osu", buf, 2000000, true);
         * if (ctx.err()) {
         *     console.error(ctx.err());
         *     process.exit(1);
         * }
         */
        static void Err(const FunctionCallbackInfo<Value>& args)
        {
            Isolate* iso = args.GetIsolate();
            oppai_ctx* c = &node::ObjectWrap::Unwrap<Ctx>(args.Holder())->ctx;

            if (oppai_err(c)) {
                args.GetReturnValue()
                    .Set(String::NewFromUtf8(iso, oppai_err(c)));
            }
        }

        // defined after Beatmap
        static void PpCalc(const FunctionCallbackInfo<Value>& args);
        static void PpCalcAcc(const FunctionCallbackInfo<Value>& args);
    };

    Persistent<Function> Ctx::constructor;

    // -------------------------------------------------------------------------

    class Buffer : public node::ObjectWrap
    {
    public:
        char* buf;

		/**
         * **`oppai.Buffer`**
         *
         * Allocates a new buffer to be used for beatmap parsing.
         * This should be at least as big as the .osu files you're going to
         * parse.
         *
         * @class Buffer
         * @param {uint32} nbytes - number of bytes to allocate
         * @example
         * buf = oppai.Buffer(2000000)
         */
        static void Init(Local<Object> exports)
        {
            Isolate* isolate = exports->GetIsolate();
            Local<FunctionTemplate> t = FunctionTemplate::New(isolate, New);

            t->SetClassName(String::NewFromUtf8(isolate, "Buffer"));
            t->InstanceTemplate()->SetInternalFieldCount(1);

            constructor.Reset(isolate, t->GetFunction());
            exports->Set(
                String::NewFromUtf8(isolate, "Buffer"),
                t->GetFunction()
            );
        }

        static void NewInstance(
            const FunctionCallbackInfo<Value>& args,
            Isolate* isolate)
        {
            const int argc = 1;
            Local<Value> argv[argc] = { args[0] };

            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Context> context = isolate->GetCurrentContext();
            Local<Object> instance =
                cons->NewInstance(context, argc, argv).ToLocalChecked();

            args.GetReturnValue().Set(instance);
        }

        static void NewInstance(const FunctionCallbackInfo<Value>& args)
        {
            Isolate* isolate = args.GetIsolate();
            NewInstance(args, isolate);
        }

    private:
        static Persistent<Function> constructor;

        explicit Buffer(u32 nbytes)
            : buf((char*)malloc(nbytes))
        {
            ndbgprintf("allocated buffer %p (%" fu32 " bytes)\n", buf, nbytes);
        }

        ~Buffer()
        {
            ndbgprintf("deallocating buffer %p\n", buf);
            free(buf);
        }

        static void New(const FunctionCallbackInfo<Value>& args)
        {
            Isolate* isolate = args.GetIsolate();

            if (args.IsConstructCall())
            {
                u32 nbytes = (f32)args[0]->Uint32Value();
                Buffer* c = new Buffer(nbytes);
                c->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            }
            else {
                NewInstance(args, isolate);
            }
        }
    };

    Persistent<Function> Buffer::constructor;

    // -------------------------------------------------------------------------

    class Beatmap : public node::ObjectWrap
    {
    public:
        beatmap b;

		/**
         * **`oppai.Beatmap`**
         *
         * Creates an empty Beatmap object. See 'Beatmap.parse' to fill this
         * object.
         *
         * @class Beatmap
         */
        static void Init(Local<Object> exports)
        {
            Isolate* isolate = exports->GetIsolate();
            Local<FunctionTemplate> t = FunctionTemplate::New(isolate, New);

            t->SetClassName(String::NewFromUtf8(isolate, "Beatmap"));
            t->InstanceTemplate()->SetInternalFieldCount(1);

            NODE_SET_PROTOTYPE_METHOD(t, "artist", Artist);
            NODE_SET_PROTOTYPE_METHOD(t, "title", Title);
            NODE_SET_PROTOTYPE_METHOD(t, "version", Version);
            NODE_SET_PROTOTYPE_METHOD(t, "creator", Creator);
            NODE_SET_PROTOTYPE_METHOD(t, "cs", Cs);
            NODE_SET_PROTOTYPE_METHOD(t, "od", Od);
            NODE_SET_PROTOTYPE_METHOD(t, "ar", Ar);
            NODE_SET_PROTOTYPE_METHOD(t, "hp", Hp);
            NODE_SET_PROTOTYPE_METHOD(t, "numObjects", NumObjects);
            NODE_SET_PROTOTYPE_METHOD(t, "numCircles", NumCircles);
            NODE_SET_PROTOTYPE_METHOD(t, "numSliders", NumSliders);
            NODE_SET_PROTOTYPE_METHOD(t, "numSpinners", NumSpinners);
            NODE_SET_PROTOTYPE_METHOD(t, "maxCombo", MaxCombo);

            NODE_SET_PROTOTYPE_METHOD(t, "setCs", SetCs);
            NODE_SET_PROTOTYPE_METHOD(t, "setOd", SetOd);
            NODE_SET_PROTOTYPE_METHOD(t, "setAr", SetAr);

            NODE_SET_PROTOTYPE_METHOD(t, "applyMods", ApplyMods);
            NODE_SET_PROTOTYPE_METHOD(t, "parse", Parse);

            constructor.Reset(isolate, t->GetFunction());
            exports->Set(
                String::NewFromUtf8(isolate, "Beatmap"),
                t->GetFunction()
            );
        }

        static void NewInstance(
            const FunctionCallbackInfo<Value>& args,
            Isolate* isolate)
        {
            const int argc = 1;
            Local<Value> argv[argc] = { args[0] };

            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Context> context = isolate->GetCurrentContext();
            Local<Object> instance =
                cons->NewInstance(context, argc, argv).ToLocalChecked();

            args.GetReturnValue().Set(instance);
        }

        static void NewInstance(const FunctionCallbackInfo<Value>& args)
        {
            Isolate* isolate = args.GetIsolate();
            NewInstance(args, isolate);
        }

    private:
        static Persistent<Function> constructor;

        explicit Beatmap(oppai_ctx* ctx) : b(ctx) {
            ndbgprintf("allocated beatmap %p with ctx %p\n", &b, ctx);
        }

        ~Beatmap() {
            ndbgprintf("deallocating beatmap %p\n", &b);
        }

        static void New(const FunctionCallbackInfo<Value>& args)
        {
            Isolate* isolate = args.GetIsolate();

            if (args.IsConstructCall())
            {
                oppai_ctx* ctx =
                    &node::ObjectWrap::Unwrap<Ctx>(args[0]->ToObject())->ctx;

                Beatmap* b = new Beatmap(ctx);
                b->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            }
            else {
                NewInstance(args, isolate);
            }
        }

		/**
         * Applies map-changing mods. Note that this is currently not reversible
         * and you will have to re-parse the map to undo mods like DT.
         *
         * @param {uint32} usedMods - the mods bitmask, which can be built by
         *                            bit-wise OR-ing the mod constants
         *
         * @name applyMods
         * @memberof Beatmap
         * @static
         *
         * @example
         * var ctx = oppai.Ctx();
         * var b = oppai.Beatmap(ctx);
         * var buf = oppai.Buffer(2000000);
         * b.parse("some_file.osu", buf, 2000000, true);
         * b.applyMods(oppai.hd | oppai.hr);
         */
        static void ApplyMods(const FunctionCallbackInfo<Value>& args)
        {
            beatmap* b = &node::ObjectWrap::Unwrap<Beatmap>(args.Holder())->b;
            u32 mods_mask = args[0]->Uint32Value();
            ndbgprintf("beatmap<%p>->apply_mods(%08X)\n", b, mods_mask);
            b->apply_mods(mods_mask);
        }

		/**
         * Parse .osu file into a 'oppai.Beatmap' object.
         *
         * @param {string} osuFile - path to the .osu file. If "-" is passed,
         *                           input will be read from stdin.
         * @param {boolean} [disableCache=false] -
         *     by default, beatmaps are cached in a pre-parsed binary format
         *     to disk. Setting this to true disables this feature.
         * @param {string} [customCacheFolder="path/to/current/executable"] -
         *     overrides the base cache directory.
         *
         * @name parse
         * @memberof Beatmap
         * @static
         *
         * @example
         * var ctx = oppai.Ctx();
         * var b = oppai.Beatmap(ctx);
         * var buf = oppai.Buffer(2000000);
         * b.parse("some_file.osu", buf, 2000000, true);
         */
        static void Parse(const FunctionCallbackInfo<Value>& args)
        {
            beatmap* b = &node::ObjectWrap::Unwrap<Beatmap>(args.Holder())->b;

            std::string osu_file(*String::Utf8Value(args[0]->ToString()));
            std::string custom_cache_folder;

            char* buf =
                node::ObjectWrap::Unwrap<Buffer>(
                    args[1]->ToObject()
                )->buf;

            if (args[4]->IsString()) {
                custom_cache_folder =
                    std::string(*String::Utf8Value(args[4]->ToString()));
            }

            ndbgprintf(
                "beatmap::parse(\"%s\", %p, %p, %" fu32 ", %s, \"%s\")\n",
                osu_file.c_str(),
                b,
                buf,
                args[2]->Uint32Value(),
                args[3]->IsBoolean() && args[3]->IsTrue() ? "true" : "false",
                custom_cache_folder.length() ?
                    custom_cache_folder.c_str()
                    : "(null)"
            );

            beatmap::parse(
                osu_file.c_str(),
                *b,
                buf,
                args[2]->Uint32Value(),
                args[3]->IsBoolean() && args[3]->IsTrue(),
                custom_cache_folder.length() ?
                    custom_cache_folder.c_str()
                    : 0
            );
        }

#define STR_GETTER(getter_name, field_name)                                    \
        static void getter_name(const FunctionCallbackInfo<Value>& args)       \
        {                                                                      \
            Isolate* iso = args.GetIsolate();                                  \
            beatmap* b = &node::ObjectWrap::Unwrap<Beatmap>(args.Holder())->b; \
            args.GetReturnValue().Set(String::NewFromUtf8(iso, b->field_name));\
        }

        STR_GETTER(Artist, artist)
        STR_GETTER(Title, title)
        STR_GETTER(Version, version)
        STR_GETTER(Creator, creator)

#undef STR_GETTER

#define DOUBLE_GETTER(getter_name, field_name)                                 \
        static void getter_name(const FunctionCallbackInfo<Value>& args)       \
        {                                                                      \
            Isolate* iso = args.GetIsolate();                                  \
            beatmap* b = &node::ObjectWrap::Unwrap<Beatmap>(args.Holder())->b; \
            args.GetReturnValue().Set(Number::New(iso, b->field_name));        \
        }

        DOUBLE_GETTER(Cs, cs)
        DOUBLE_GETTER(Od, od)
        DOUBLE_GETTER(Ar, ar)
        DOUBLE_GETTER(Hp, hp)

#undef DOUBLE_GETTER

#define INT_GETTER(getter_name, field_name)                                    \
        static void getter_name(const FunctionCallbackInfo<Value>& args)       \
        {                                                                      \
            Isolate* iso = args.GetIsolate();                                  \
            beatmap* b = &node::ObjectWrap::Unwrap<Beatmap>(args.Holder())->b; \
            args.GetReturnValue()                                              \
                .Set(Uint32::NewFromUnsigned(iso, b->field_name));             \
        }

        INT_GETTER(NumObjects, num_objects)
        INT_GETTER(NumCircles, num_circles)
        INT_GETTER(NumSliders, num_sliders)
        INT_GETTER(NumSpinners, num_spinners)
        INT_GETTER(MaxCombo, max_combo)

#undef INT_GETTER

#define FLOAT_SETTER(setter_name, field_name)                                  \
        static void Set##setter_name(const FunctionCallbackInfo<Value>& args)  \
        {                                                                      \
            beatmap* b = &node::ObjectWrap::Unwrap<Beatmap>(args.Holder())->b; \
            b->field_name = (f32)args[0]->NumberValue();                       \
        }

        FLOAT_SETTER(Cs, cs)
        FLOAT_SETTER(Od, od)
        FLOAT_SETTER(Ar, ar)
        FLOAT_SETTER(Hp, hp)

#undef FLOAT_SETTER
    };

    Persistent<Function> Beatmap::constructor;

    // -------------------------------------------------------------------------

    internalfn
    void ReturnPp(
        pp_calc_result& ppres,
        const FunctionCallbackInfo<Value>& args,
        Isolate* iso)
    {
        Local<Object> res = Object::New(iso);

        res->Set(
            String::NewFromUtf8(iso, "accPercent"),
            Number::New(iso, ppres.acc_percent)
        );

        res->Set(String::NewFromUtf8(iso, "pp"), Number::New(iso, ppres.pp));

        res->Set(
            String::NewFromUtf8(iso, "aimPp"),
            Number::New(iso, ppres.aim_pp)
        );

        res->Set(
            String::NewFromUtf8(iso, "speedPp"),
            Number::New(iso, ppres.speed_pp)
        );

        res->Set(
            String::NewFromUtf8(iso, "accPp"),
            Number::New(iso, ppres.acc_pp)
        );

        args.GetReturnValue().Set(res);
    }

	/**
     * Calculates ppv2
     *
     * @param {number} aim - aim difficulty (see 'DiffCalcCtx.diffCalc')
     * @param {number} speed - speed difficulty (see 'DiffCalcCtx.diffCalc')
     * @param {Object<oppai.Beatmap>} b - the beatmap with map-modifying
     *                                    mods already applied
     * @param {uint32} [usedMods=oppai.nomod] - the mods bitmask
     *                                          (see 'Beatmap.applyMods')
     * @param {uint16} [combo=0xFFFF] - desired combo. 0xFFFF will assume full
     *                                  combo
     * @param {uint16} [misses=0] - amount of misses
     * @param {uint16} [c300=0xFFFF] - amount of 300s. 0xFFFF will automatically
     *                                 calculate this value based on the number
     *                                 of misses, 100s and 50s.
     * @param {uint16} [c100=0] - number of 100s
     * @param {uint16} [c50=0] - number of 50s
     * @param {uint32} [scoreVersion=1] - 1 or 2, affects accuracy pp
     *
     * @name ppCalc
     * @memberof Ctx
     * @static
     * @returns {object} an object containing 'accPercent', 'pp', 'accPp',
     *                   'speedPp' and 'aimPp'.
     * @example
     * var ctx = oppai.Ctx();
     *
     * var b = oppai.Beatmap(ctx);
     * var buf = oppai.Buffer(2000000);
     * b.parse("some_file.osu", buf, 2000000, true);
     *
     * dctx = oppai.DiffCalcCtx(ctx);
     * diff = dctx.diffCalc(b);
     * res = ctx.ppCalc(diff.aim, diff.speed, b);
     * console.log(
     *     util.format(
     *         "\n%d aim\n%d speed\n%d acc\n%d pp\nfor %d%%",
     *         res.aimPp, res.speedPp, res.accPp, res.pp, res.accPercent
     *     )
     * );
     */
    void Ctx::PpCalc(const FunctionCallbackInfo<Value>& args)
    {
        Isolate* iso = args.GetIsolate();
        oppai_ctx* c = &node::ObjectWrap::Unwrap<Ctx>(args.Holder())->ctx;

        beatmap* b =
            &node::ObjectWrap::Unwrap<Beatmap>(args[2]->ToObject())->b;

        pp_calc_result ppres =
            pp_calc(
                c,
                args[0]->NumberValue(),
                args[1]->NumberValue(),
                *b,
                args[3]->IsUint32() ? args[3]->Uint32Value() : mods::nomod,
                args[4]->IsUint32() ? (u16)args[4]->Uint32Value() : 0xFFFF,
                args[5]->IsUint32() ? (u16)args[5]->Uint32Value() : 0,
                args[6]->IsUint32() ? (u16)args[6]->Uint32Value() : 0xFFFF,
                args[7]->IsUint32() ? (u16)args[7]->Uint32Value() : 0,
                args[8]->IsUint32() ? (u16)args[8]->Uint32Value() : 0,
                args[9]->IsUint32() ? (u16)args[9]->Uint32Value() : 1
            );

        ndbgprintf(
            "pp_calc(%p, %.17g, %.17g, %p, %08X, %" fu16
            ", %" fu16 ", %" fu16 ", %" fu16 ", %" fu16 ", %" fu16 ")\n",
            c,
            args[0]->NumberValue(),
            args[1]->NumberValue(),
            b,
            args[3]->IsUint32() ? args[3]->Uint32Value() : mods::nomod,
            args[4]->IsUint32() ? (u16)args[4]->Uint32Value() : 0xFFFF,
            args[5]->IsUint32() ? (u16)args[5]->Uint32Value() : 0,
            args[6]->IsUint32() ? (u16)args[6]->Uint32Value() : 0xFFFF,
            args[7]->IsUint32() ? (u16)args[7]->Uint32Value() : 0,
            args[8]->IsUint32() ? (u16)args[8]->Uint32Value() : 0,
            args[9]->IsUint32() ? (u16)args[9]->Uint32Value() : 1
        );

        ReturnPp(ppres, args, iso);
    }

	/**
     * Same as 'Ctx.ppCalc' but uses accuracy percentage instead of number of
     * 100/50s. Accuracy is automatically rounded to the closest 100/50 count.
     *
     * @param {number} aim - aim difficulty (see 'DiffCalcCtx.diffCalc')
     * @param {number} speed - speed difficulty (see 'DiffCalcCtx.diffCalc')
     * @param {Object<oppai.Beatmap>} b - the beatmap with map-modifying
     *                                    mods already applied
     * @param {number} [accuracyPercent=100.0] - the accuracy in percentage
     * @param {uint32} [usedMods=oppai.nomod] - the mods bitmask
     *                                          (see 'Beatmap.applyMods')
     * @param {uint16} [combo=0xFFFF] - desired combo. 0xFFFF will assume full
     *                                  combo
     * @param {uint16} [misses=0] - amount of misses
     * @param {uint32} [scoreVersion=1] - 1 or 2, affects accuracy pp
     *
     * @name ppCalcAcc
     * @memberof Ctx
     * @static
     * @returns {object} an object containing 'accPercent', 'pp', 'accPp',
     *                   'speedPp' and 'aimPp'.
     * @example
     * var ctx = oppai.Ctx();
     *
     * var b = oppai.Beatmap(ctx);
     * var buf = oppai.Buffer(2000000);
     * b.parse("some_file.osu", buf, 2000000, true);
     *
     * dctx = oppai.DiffCalcCtx(ctx);
     * diff = dctx.diffCalc(b);
     * res = ctx.ppCalc(diff.aim, diff.speed, b, 95.00);
     * console.log(
     *     util.format(
     *         "\n%d aim\n%d speed\n%d acc\n%d pp\nfor %d%%",
     *         res.aimPp, res.speedPp, res.accPp, res.pp, res.accPercent
     *     )
     * );
     */
    void Ctx::PpCalcAcc(const FunctionCallbackInfo<Value>& args)
    {
        Isolate* iso = args.GetIsolate();
        oppai_ctx* c = &node::ObjectWrap::Unwrap<Ctx>(args.Holder())->ctx;

        beatmap* b =
            &node::ObjectWrap::Unwrap<Beatmap>(args[2]->ToObject())->b;

        pp_calc_result ppres =
            pp_calc_acc(
                c,
                args[0]->NumberValue(),
                args[1]->NumberValue(),
                *b,
                args[3]->IsNumber() ? args[3]->NumberValue() : 100.0,
                args[4]->IsUint32() ? args[4]->Uint32Value() : mods::nomod,
                args[5]->IsUint32() ? (u16)args[5]->Uint32Value() : 0xFFFF,
                args[6]->IsUint32() ? (u16)args[6]->Uint32Value() : 0,
                args[7]->IsUint32() ? (u16)args[7]->Uint32Value() : 1
            );

        ndbgprintf(
            "pp_calc_acc(%p, %.17g, %.17g, %p, %.17g, %08X, %" fu16
            ", %" fu16 ", %" fu32 ")\n",
            c,
            args[0]->NumberValue(),
            args[1]->NumberValue(),
            b,
            args[3]->IsNumber() ? args[3]->NumberValue() : 100.0,
            args[4]->IsUint32() ? args[4]->Uint32Value() : mods::nomod,
            args[5]->IsUint32() ? (u16)args[5]->Uint32Value() : 0xFFFF,
            args[6]->IsUint32() ? (u16)args[6]->Uint32Value() : 0,
            args[7]->IsUint32() ? args[7]->Uint32Value() : 1
        );

        ReturnPp(ppres, args, iso);
    }

    // -------------------------------------------------------------------------

    class DiffCalcCtx : public node::ObjectWrap
    {
    public:
        d_calc_ctx ctx;

		/**
         * **`oppai.DiffCalcCtx`**
         *
         * Creates a difficulty calculation context for the current thread.
         * Each instance should not be used concurrently.
         *
         * @class DiffCalcCtx
         */
        static void Init(Local<Object> exports)
        {
            Isolate* isolate = exports->GetIsolate();
            Local<FunctionTemplate> t = FunctionTemplate::New(isolate, New);

            t->SetClassName(String::NewFromUtf8(isolate, "DiffCalcCtx"));
            t->InstanceTemplate()->SetInternalFieldCount(1);

            NODE_SET_PROTOTYPE_METHOD(t, "diffCalc", DiffCalc);

            constructor.Reset(isolate, t->GetFunction());
            exports->Set(
                String::NewFromUtf8(isolate, "DiffCalcCtx"),
                t->GetFunction()
            );
        }

        static void NewInstance(
            const FunctionCallbackInfo<Value>& args,
            Isolate* isolate)
        {
            const int argc = 1;
            Local<Value> argv[argc] = { args[0] };

            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Context> context = isolate->GetCurrentContext();
            Local<Object> instance =
                cons->NewInstance(context, argc, argv).ToLocalChecked();

            args.GetReturnValue().Set(instance);
        }

        static void NewInstance(const FunctionCallbackInfo<Value>& args)
        {
            Isolate* isolate = args.GetIsolate();
            NewInstance(args, isolate);
        }

    private:
        static Persistent<Function> constructor;

        explicit DiffCalcCtx(oppai_ctx* ctx) : ctx(ctx) {
            ndbgprintf("allocated d_calc_ctx %p\n", &ctx);
        }

        ~DiffCalcCtx() {
            ndbgprintf("deallocating d_calc_ctx %p\n", &ctx);
        }

        static void New(const FunctionCallbackInfo<Value>& args)
        {
            Isolate* isolate = args.GetIsolate();

            if (args.IsConstructCall())
            {
                oppai_ctx* ctx =
                    &node::ObjectWrap::Unwrap<Ctx>(args[0]->ToObject())->ctx;

                DiffCalcCtx* c = new DiffCalcCtx(ctx);
                c->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            }
            else {
                NewInstance(args, isolate);
            }
        }

		/**
         * Calculates overall, aim and speed stars for a map.
         *
         * @param {object<oppai.Beatmap>} b - a parsed beatmap with map-changing
         *                                    mods already applied.
         * @param {boolean} [withRhythmAwkwardness=false] -
         *     calculates and adds rhythm awkwardness to the returned object.
         * @param {boolean} [withNSingles=false] -
         *     calculates and adds the number of spacing singletaps (as seen by
         *     the difficulty calculator) to the returned object. These are
         *     based on spacing thresholds.
         * @param {boolean} [withNSinglesTiming=false] -
         *     calculates and adds the number of timing singletaps (1/2 notes)
         *     to the returned object.
         * @param {boolean} [withNSinglesThreshold=false] -
         *     calculates and adds the number of notes that are 1/2 or slower at
         *     singletapThreshold BPM.
         * @param {uint32} [singletapThreshold=240] -
         *     singletap threshold BPM for withNSinglesThreshold
         *
         * @returns {object} an object containing 'stars', 'aim', 'speed' and,
         *     optionally, 'rhythmAwkwardness', 'nSingles', 'nSinglesTiming' and
         *     'nSinglesThreshold'.
         *
         * @name DiffCalc
         * @memberof DiffCalcCtx
         * @static
         *
         * @example
         * var ctx = oppai.Ctx();
         *
         * var b = oppai.Beatmap(ctx);
         * var buf = oppai.Buffer(2000000);
         * b.parse("some_file.osu", buf, 2000000, true);
         *
         * dctx = oppai.DiffCalcCtx(ctx);
         * diff = dctx.diffCalc(b);
         * console.log(
         *     util.format(
         *         "\n%d stars\n%d aim stars\n%d speed stars",
         *         diff.stars, diff.aim, diff.speed
         *     )
         * );
         */
        static void DiffCalc(const FunctionCallbackInfo<Value>& args)
        {
            Isolate* iso = args.GetIsolate();

            d_calc_ctx* c =
                &node::ObjectWrap::Unwrap<DiffCalcCtx>(args.Holder())->ctx;

            beatmap* b =
                &node::ObjectWrap::Unwrap<Beatmap>(args[0]->ToObject())->b;

            f64 aim = 0, speed = 0, rhythm_awkwardness = 0;
            u16 nsingles = 0, nsingles_timing = 0, nsingles_threshold = 0;
            i32 singletap_threshold = 240;

            if (args[5]->IsInt32()) {
                singletap_threshold = args[5]->Int32Value();
            }

            ndbgprintf(
                "d_calc(%p, %p, %p, %p, %p, %p, %p, %p, %" fi32 ")\n",
                c, b, &aim, &speed,
                args[1]->IsTrue() ? &rhythm_awkwardness : 0,
                args[2]->IsTrue() ? &nsingles : 0,
                args[3]->IsTrue() ? &nsingles_timing : 0,
                args[4]->IsTrue() ? &nsingles_threshold : 0,
                singletap_threshold
            );

            f64 stars =
                d_calc(
                    c, *b, &aim, &speed,
                    args[1]->IsTrue() ? &rhythm_awkwardness : 0,
                    args[2]->IsTrue() ? &nsingles : 0,
                    args[3]->IsTrue() ? &nsingles_timing : 0,
                    args[4]->IsTrue() ? &nsingles_threshold : 0,
                    singletap_threshold
                );

            Local<Object> res = Object::New(iso);

            res->Set(
                String::NewFromUtf8(iso, "stars"),
                Number::New(iso, stars)
            );

            res->Set(String::NewFromUtf8(iso, "aim"), Number::New(iso, aim));

            res->Set(
                String::NewFromUtf8(iso, "speed"),
                Number::New(iso, speed)
            );

            if (args[1]->IsTrue())
            {
                res->Set(
                    String::NewFromUtf8(iso, "rhythmAwkwardness"),
                    Number::New(iso, stars)
                );
            }

            if (args[2]->IsTrue())
            {
                res->Set(
                    String::NewFromUtf8(iso, "nSingles"),
                    Uint32::NewFromUnsigned(iso, nsingles)
                );
            }

            if (args[3]->IsTrue())
            {
                res->Set(
                    String::NewFromUtf8(iso, "nSinglesTiming"),
                    Uint32::NewFromUnsigned(iso, nsingles_timing)
                );
            }

            if (args[4]->IsTrue())
            {
                res->Set(
                    String::NewFromUtf8(iso, "nSinglesThreshold"),
                    Uint32::NewFromUnsigned(iso, nsingles_threshold)
                );
            }

            args.GetReturnValue().Set(res);
        }
    };

    Persistent<Function> DiffCalcCtx::constructor;

    // -------------------------------------------------------------------------

    void InitAll(Local<Object> exports)
    {
        Ctx::Init(exports);
        Beatmap::Init(exports);
        Buffer::Init(exports);
        DiffCalcCtx::Init(exports);

        Isolate* iso = exports->GetIsolate();
        Local<Context> context = iso->GetCurrentContext();

#define mod(name)                                     \
        exports->DefineOwnProperty(                    \
            context,                                  \
            String::NewFromUtf8(iso, #name),          \
            Uint32::NewFromUnsigned(iso, mods::name), \
            v8::ReadOnly                              \
        )

        mod(nomod);
        mod(nf);
        mod(ez);
        mod(hd);
        mod(hr);
        mod(dt);
        mod(ht);
        mod(nc);
        mod(fl);
        mod(so);

#undef mod
    }

    NODE_MODULE(oppai, InitAll)
}
