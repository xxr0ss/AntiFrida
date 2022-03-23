#include <jni.h>

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_xxr0ss_antifrida_utils_AntiFridaUtil_checkFridaByModuleEnum(JNIEnv *env, jobject thiz,
                                                                     jstring target) {
    return JNI_TRUE;
}