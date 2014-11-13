#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "screenshot.h"
#include "fb2png.h"
#include "log.h"

#ifndef WIN32

static int getFrameBuffer(JNIEnv * env, jclass cls){
	int ret = 0;
	char* path = "/sdcard/screenshot.png";
	ret = fb2png(path);
	if (!ret) {
		I("I Image saved to : %s", path);
		E("I Image saved to : %s", path);
	}
	return ret;
}

// *****************register methods******************
int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = (*env)->FindClass(env, className);

    if (clazz == NULL) {
        return JNI_FALSE;
    }

    if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

int init_Exec(JNIEnv *env) {
	if (!registerNativeMethods(env, classPathName, methods,
			sizeof(methods) / sizeof(methods[0]))) {
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    JNIEnv* env = NULL;

    if ((*vm)->GetEnv(vm, (void**)&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }

    env = uenv.env;

    if (init_Exec(env) != JNI_TRUE) {
        return -1;
    }

    return JNI_VERSION_1_4;
}

#endif//#ifndef WIN32
