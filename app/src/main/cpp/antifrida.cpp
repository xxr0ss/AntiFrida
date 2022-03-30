#include <jni.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <android/log.h>


extern "C" int my_read(int, void *, int);

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_xxr0ss_antifrida_utils_AntiFridaUtil_checkFridaByPort(JNIEnv *env, jobject thiz,
                                                               jint port) {
    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    inet_aton("127.0.0.1", &sa.sin_addr);
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(sock, (struct sockaddr *) &sa, sizeof(sa)) == 0) {
        // we can connect to frida-server when it's running
        close(sock);
        return JNI_TRUE;
    }

    return JNI_FALSE;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_xxr0ss_antifrida_utils_AntiFridaUtil_testMyRead(JNIEnv *env, jobject thiz) {
    char buf[32];
    int fd;
    if ((fd = openat(AT_FDCWD, "/proc/self/maps", O_RDONLY, 0)) > 0) {
        int size = my_read(fd, buf, 31);
        if (size == 0) {
            __android_log_print(ANDROID_LOG_INFO, "JNI", "read 0 bytes", buf);
        } else {
            buf[size] = 0;
            __android_log_print(ANDROID_LOG_INFO, "JNI", "data %d: %s", size, buf);
        }
    } else {
        __android_log_print(ANDROID_LOG_INFO, "JNI", "openat error %d", errno);
    }
}