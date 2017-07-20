#include "dp_oppai_Buffer.h"
#include <stdlib.h>

JNIEXPORT void JNICALL
Java_dp_oppai_Buffer_dispose(JNIEnv* env, jobject obj) {
	char* p = getHandle<char>(env, obj);
	if (!p)
		return;
	free(p);
}

JNIEXPORT void JNICALL
Java_dp_oppai_Buffer_newBuffer(JNIEnv *env, jobject obj, jint length) {
	char* buffer = (char*)malloc(length);
	setHandle(env, obj, buffer);
}
