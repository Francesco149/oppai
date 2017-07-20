#include "dp_oppai_PPCalc.h"

JNIEXPORT void JNICALL
Java_dp_oppai_PPCalc_nativeCalcPP(JNIEnv* env, jobject obj, jobject beatmapObj, jobject ctxObject, jdouble aim, jdouble speed, jint usedMods, jint combo, jint misses, jint c300, jint c100, jint c50, jint scoreVersion) {
	f64 _aim = (f64)aim, _speed = (f64)speed;
	u32 _usedMods = (u32)usedMods;
	u16 _combo = (u16)combo, _misses = (u16)misses, _c300 = (u16)c300, _c100 = (u16)c100, _c50 = (u16)c50;
	u32 _scoreVersion = (u32)scoreVersion;

	oppai_ctx* ctx = getHandle<oppai_ctx>(env, ctxObject);
	beatmap* b = getHandle<beatmap>(env, beatmapObj);

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
Java_dp_oppai_PPCalc_nativeCalcPPAcc(JNIEnv* env, jobject obj, jobject beatmapObj, jobject ctxObject, jdouble aim, jdouble speed, jdouble acc, jint usedMods, jint combo, jint misses, jint scoreVersion) {
	f64 _aim = (f64)aim,
		_speed = (f64)speed,
		accPercent = (f64)acc;
	u32 _usedMods = (u32)usedMods,
		_scoreVersion = (u32)scoreVersion;
	u16 _combo = (u16)combo,
		_misses = (u16)misses;

	oppai_ctx* ctx = getHandle<oppai_ctx>(env, ctxObject);
	beatmap* b = getHandle<beatmap>(env, beatmapObj);

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