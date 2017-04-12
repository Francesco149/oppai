class v2f
{
public:
    f32 x, y;

#if defined(_DEBUG) || defined(SHOW_BEATMAP)
#define i() memset(buf, 0, sizeof(buf))
#else
#define i()
#endif

    v2f(f32 x, f32 y) : x(x), y(y) { i(); }
    v2f()             : x(0), y(0) { i(); }
    v2f(f32 v)        : x(v), y(v) { i(); }
#undef i

#if defined(_DEBUG) || defined(SHOW_BEATMAP)
    const char* str()
    {
        sprintf(buf, "(%g %g)", x, y);
        return buf;
    }
#endif

    f32 len() const {
        return sqrt(x * x + y * y);
    }

#define do_op(o) \
    inline void operator o##= (const v2f& v) { x o##= v.x; y o##= v.y; }       \
    inline void operator o##= (f32 f)        { x o##= f; y o##= f; }           \
    v2f operator o (const v2f& v) const      { return v2f(x o v.x, y o v.y); } \
    v2f operator o (f32 f) const             { return v2f(x o f, y o f); }

    do_op(+)
    do_op(-)
    do_op(*)
    do_op(/)

#undef do_op

#if defined(_DEBUG) || defined(SHOW_BEATMAP)
protected:
    // this is used for formatting with str()
    // without having to pass copies of the string around
    // obviously not thread safe
    static char buf[42];
};

char v2f::buf[42];
#else
};
#endif

