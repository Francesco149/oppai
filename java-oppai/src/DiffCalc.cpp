#include "dp_oppai_DiffCalc.h"

JNIEXPORT void JNICALL
Java_dp_oppai_DiffCalc_nativeDiffCalc(JNIEnv *env, jobject obj, jobject beatmapObj, jobject diffCtxObject, jboolean withAwkwardness, jboolean withAimSingles, jboolean withTimingSingles, jboolean withThresholdSingles, jint singletapThreshold) {
	int with_awkwardness = (int)withAwkwardness;
	int with_aim_singles = (int)withAimSingles;
	int with_timing_singles = (int)withTimingSingles;
	int with_threshold_singles = (int)withThresholdSingles;
	i32 singletap_threshold = (int)singletapThreshold; // The default value will be passed from singletapThreshold if the user doesn't specify otherwise
	d_calc_ctx* dctx = getHandle<d_calc_ctx>(env, diffCtxObject);
	beatmap* b = getHandle<beatmap>(env, beatmapObj);
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