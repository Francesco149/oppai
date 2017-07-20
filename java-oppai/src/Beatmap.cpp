#include "dp_oppai_Beatmap.h"

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
