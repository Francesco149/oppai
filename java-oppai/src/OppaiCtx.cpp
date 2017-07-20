#include "dp_oppai_OppaiCtx.h"

JNIEXPORT void JNICALL
Java_dp_oppai_OppaiCtx_newCtx(JNIEnv *env, jobject obj) {
	oppai_ctx *ctx = new oppai_ctx();
	setHandle(env, obj, ctx);
}

JNIEXPORT void JNICALL
Java_dp_oppai_OppaiCtx_newDiffCalcContext(JNIEnv *env, jobject obj, jobject ctxObject) {
	oppai_ctx* ctx = getHandle<oppai_ctx>(env, ctxObject);
	d_calc_ctx* dctx = new d_calc_ctx(ctx);
	setHandle(env, obj, dctx);
}

JNIEXPORT jstring JNICALL
Java_dp_oppai_OppaiCtx_nativeGetLastErr(JNIEnv *env, jobject obj) {

	oppai_ctx* ctx = getHandle<oppai_ctx>(env, obj);

	char const* error = "";
	if (oppai_err(ctx))
		error = oppai_err(ctx);

	return env->NewStringUTF(error);
}

JNIEXPORT void JNICALL
Java_dp_oppai_OppaiCtx_dispose(JNIEnv* env, jobject obj) {
	oppai_ctx* ctx = getHandle<oppai_ctx>(env, obj);
	if (!ctx)
		return;
	delete ctx;
}
