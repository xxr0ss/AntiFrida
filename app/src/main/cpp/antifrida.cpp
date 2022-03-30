#include <jni.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <android/log.h>
#include <sys/stat.h>
#include <cstdlib>
#include <string>

#define unused_param(x) (x)

const char MAPS_FILE[] = "/proc/self/maps";

extern "C" int my_read(int, void *, int);

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_xxr0ss_antifrida_utils_AntiFridaUtil_checkFridaByPort(JNIEnv *env, jobject thiz,
                                                               jint port) {
    unused_param(thiz);
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
JNIEXPORT jstring JNICALL
Java_com_xxr0ss_antifrida_utils_AntiFridaUtil_nativeGetProcMaps(JNIEnv *env, jobject thiz, jboolean useCustomizedSyscall) {
    // check /proc/self/maps file size
    int fd = openat(AT_FDCWD, MAPS_FILE, O_RDONLY, 0);

    int buf_increase_size = 1024;
    int buf_size = buf_increase_size;
    char *readBuf = (char *) malloc(buf_size);
    int total_read = 0;

    while (true) {
        ssize_t read_size = useCustomizedSyscall ?
                read(fd, readBuf + total_read, buf_increase_size)
                : my_read(fd, readBuf + total_read, buf_increase_size);
        total_read += (int) read_size;
        if (!total_read) {
            // read failed
            break;
        }
        if (read_size < buf_increase_size)
            break;
        buf_size += buf_increase_size;
        readBuf = (char *) realloc(readBuf, buf_size);
    }
    close(fd);

    if (total_read == 0) {
        // for the issue mentioned https://github.com/android/ndk/issues/1422
        // our customized syscall currently ignored the call to __set_errno_internal()
        __android_log_print(ANDROID_LOG_INFO, "JNI", "read 0 bytes, errno %s: %d",
                            useCustomizedSyscall? "unknown" : strerror(errno),
                            useCustomizedSyscall? 0 : errno);
        return nullptr;
    }
    readBuf[total_read] = 0;

    jstring res = env->NewStringUTF(readBuf);
    free(readBuf);
    return res;
}
