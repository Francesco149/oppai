// This is the main file of the dll
#include "dp_oppai_Beatmap.h"
#include "../../main.cc"
#include "handle.h"
#include <iostream>
#include "OppaiCtx.cpp"
#include "Buffer.cpp"

JNIEXPORT void JNICALL
Java_dp_oppai_Beatmap_dispose(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	if (!b)
		return;
	delete b;
}

JNIEXPORT void JNICALL
Java_dp_oppai_Beatmap_newBeatmap(JNIEnv *env, jobject obj, jobject ctxObject) {
	oppai_ctx* ctx = getHandle<oppai_ctx>(env, ctxObject);
	beatmap* b = new beatmap(ctx);
	setHandle(env, obj, b);
}

JNIEXPORT void JNICALL
Java_dp_oppai_Beatmap_nativeParse(JNIEnv *env, jobject obj, jstring osuFilePath, jobject bufferObj, jint bufferLength, jboolean disableCaching, jstring cacheFilePath) {
	beatmap* b = getHandle<beatmap>(env, obj);
	char* buffer = getHandle<char>(env, bufferObj);
	u32 bufLen = (u32)bufferLength;
	bool disableCache = (bool)disableCaching;
	char const* customCachePath = env->GetStringUTFChars(cacheFilePath, 0);;
	char const* filePath = env->GetStringUTFChars(osuFilePath, 0);

	beatmap::parse(filePath, *b, buffer, bufLen, disableCache, customCachePath);
}

JNIEXPORT void JNICALL
Java_dp_oppai_Beatmap_nativeDiffCalc(JNIEnv *env, jobject obj, jobject diffCtxObject, jboolean withAwkwardness, jboolean withAimSingles, jboolean withTimingSingles, jboolean withThresholdSingles, jint singletapThreshold) {
	int with_awkwardness = (int)withAwkwardness;
	int with_aim_singles = (int)withAimSingles;
	int with_timing_singles = (int)withTimingSingles;
	int with_threshold_singles = (int)withThresholdSingles;
	i32 singletap_threshold = (int)singletapThreshold; // The default value will be passed from singletapThreshold if the user doesn't specify otherwise
	d_calc_ctx* dctx = getHandle<d_calc_ctx>(env, diffCtxObject);
	beatmap* b = getHandle<beatmap>(env, obj);
	f64 aim = 0, speed = 0, rhythm_awkwardness = 0;
	u16 nsingles = 0, nsingles_timing = 0, nsingles_threshold = 0;

	f64 stars =
		d_calc(
			dctx, *b, &aim, &speed,
			with_awkwardness ? &rhythm_awkwardness : 0,
			with_aim_singles ? &nsingles : 0,
			with_timing_singles ? &nsingles_timing : 0,
			with_threshold_singles ? &nsingles_threshold : 0,
			singletap_threshold
		);

	jclass c = env->GetObjectClass(obj);
	jfieldID fieldID;
	jdouble value;

	// This looks like shit but there is no better way..?
#define setMember(val, fieldName)  \
	fieldID = env->GetFieldID(c, fieldName, "D"); \
	value = (jdouble)val;  \
	env->SetDoubleField(obj, fieldID, value);

	setMember(stars, "stars")
	setMember(aim, "aim")
	setMember(speed, "speed")
	setMember(rhythm_awkwardness, "rhythmAwkwardness")
	setMember(nsingles, "nSingles")
	setMember(nsingles_timing, "nSinglesTiming")
	setMember(nsingles_threshold, "nSinglesThreshold")

#undef setMember
}

JNIEXPORT jfloat JNICALL
Java_dp_oppai_Beatmap_getCS(JNIEnv *env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return b->cs;
}

JNIEXPORT jfloat JNICALL
Java_dp_oppai_Beatmap_getOD(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return b->od;
}

JNIEXPORT jfloat JNICALL
Java_dp_oppai_Beatmap_getAR(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return b->ar;
}

JNIEXPORT jfloat JNICALL
Java_dp_oppai_Beatmap_getHP(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return b->hp;
}

JNIEXPORT jstring JNICALL
Java_dp_oppai_Beatmap_getArtist(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return env->NewStringUTF(b->artist);
}

JNIEXPORT jstring JNICALL
Java_dp_oppai_Beatmap_getTitle(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return env->NewStringUTF(b->title);
}

JNIEXPORT jstring JNICALL
Java_dp_oppai_Beatmap_getVersion(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return env->NewStringUTF(b->version);
}

JNIEXPORT jstring JNICALL
Java_dp_oppai_Beatmap_getCreator(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return env->NewStringUTF(b->creator);
}

JNIEXPORT jint JNICALL 
Java_dp_oppai_Beatmap_getNumObjects(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return (int)b->num_objects;
}

JNIEXPORT jint JNICALL
Java_dp_oppai_Beatmap_getNumCircles(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return (int)b->num_circles;
}

JNIEXPORT jint JNICALL 
Java_dp_oppai_Beatmap_getNumSliders(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return (int)b->num_sliders;
}

JNIEXPORT jint JNICALL
Java_dp_oppai_Beatmap_getNumSpinners(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return (int)b->num_spinners;
}

JNIEXPORT jint JNICALL 
Java_dp_oppai_Beatmap_getMaxCombo(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return (int)b->max_combo;
}

JNIEXPORT void JNICALL
Java_dp_oppai_Beatmap_nativeCalcPP(JNIEnv* env, jobject obj, jobject ctxObject, jdouble aim, jdouble speed, jint usedMods, jint combo, jint misses, jint c300, jint c100, jint c50, jint scoreVersion) {
	f64 _aim = (f64)aim, _speed = (f64)speed;
	u32 _usedMods = (u32)usedMods;
	u16 _combo = (u16)combo, _misses = (u16)misses, _c300 = (u16)c300, _c100 = (u16)c100, _c50 = (u16)c50;
	u32 _scoreVersion = (u32)scoreVersion;

	oppai_ctx* ctx = getHandle<oppai_ctx>(env, ctxObject);
	beatmap* b = getHandle<beatmap>(env, obj);

	pp_calc_result res =
		pp_calc(
			ctx,
			_aim, _speed,
			*b,
			_usedMods,
			_combo, _misses, _c300, _c100, _c50,
			_scoreVersion
		);

	jclass c = env->GetObjectClass(obj);
	jfieldID fieldID;
	jdouble value;

#define setMember(val, fieldName)  \
	fieldID = env->GetFieldID(c, fieldName, "D"); \
	value = (jdouble)val;  \
	env->SetDoubleField(obj, fieldID, value);

	setMember(res.acc_percent, "accPercent")
	setMember(res.pp, "pp")
	setMember(res.aim_pp, "aimPP")
	setMember(res.speed_pp, "speedPP")
	setMember(res.acc_pp, "accPP")
#undef setMember
}

JNIEXPORT void JNICALL
Java_dp_oppai_Beatmap_nativeCalcPPAcc(JNIEnv* env, jobject obj, jobject ctxObject, jdouble aim, jdouble speed, jdouble acc, jint usedMods, jint combo, jint misses, jint scoreVersion) {
	f64 _aim = (f64)aim,
		_speed = (f64)speed,
		accPercent = (f64)acc;
	u32 _usedMods = (u32)usedMods,
		_scoreVersion = (u32)scoreVersion;
	u16 _combo = (u16)combo, 
		_misses = (u16)misses;

	oppai_ctx* ctx = getHandle<oppai_ctx>(env, ctxObject);
	beatmap* b = getHandle<beatmap>(env, obj);

	pp_calc_result res =
		pp_calc_acc(
			ctx,
			_aim, _speed,
			*b,
			accPercent,
			_usedMods,
			_combo, _misses,
			_scoreVersion
		);

	jclass c = env->GetObjectClass(obj);
	jfieldID fieldID;
	jdouble value;

#define setMember(val, fieldName)  \
	fieldID = env->GetFieldID(c, fieldName, "D"); \
	value = (jdouble)val;  \
	env->SetDoubleField(obj, fieldID, value);

	setMember(res.acc_percent, "accPercent")
		setMember(res.pp, "pp")
		setMember(res.aim_pp, "aimPP")
		setMember(res.speed_pp, "speedPP")
		setMember(res.acc_pp, "accPP")
#undef setMember
}

JNIEXPORT jshort JNICALL 
Java_dp_oppai_Beatmap_getMode(JNIEnv* env, jobject obj) {
	beatmap* b = getHandle<beatmap>(env, obj);
	return (jshort)b->mode;
}

JNIEXPORT void JNICALL
Java_dp_oppai_Beatmap_applyMods(JNIEnv* env, jobject obj, jint modMask) {
	u32 modBitMask = (u32)modMask;
	beatmap* b = getHandle<beatmap>(env, obj);
	b->apply_mods(modBitMask);
}

#define setter(name, funcName, varType, varJNIType)                                   \
JNIEXPORT void JNICALL                                                                \
Java_dp_oppai_Beatmap_set##funcName(JNIEnv* env, jobject obj, varJNIType valToSet) {  \
	varType value = (varType)valToSet;                                                \
	beatmap* p = getHandle<beatmap>(env, obj);                                        \
	p->name = value;                                                                  \
}

#define fsetter(name, funcName) setter(name, funcName, f32, jfloat)

	fsetter(cs, CS)
	fsetter(od, OD)
	fsetter(ar, AR)
