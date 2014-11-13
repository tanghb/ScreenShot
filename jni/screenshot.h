#include <jni.h>

#ifndef _Included_com_getpic_GetPicUsingJni
#define _Included_com_getpic_GetPicUsingJni
#ifdef __cplusplus
extern "C" {
#endif

static int getFrameBuffer(JNIEnv *env, jclass cls);

// *****************动态注册方法****************
static const char *classPathName = "com/thb/screenshot/jni/Jni";

static JNINativeMethod methods[] = {
		{"getFrameBuffer", "()I", (void*) &getFrameBuffer}
};

typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;
int init_Exec(JNIEnv *env);

int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods);
#ifdef __cplusplus
}
#endif
#endif
