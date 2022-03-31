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
const char TAG[] = "JNI";

// customized syscalls
extern "C" int my_read(int, void *, size_t);
extern "C" int
my_openat(int dirfd, const char *const __pass_object_size pathname, int flags, mode_t modes);

// Our customized __set_errno_internal for syscall.S to use.
// we do not use the one from libc due to issue https://github.com/android/ndk/issues/1422
extern "C" long __set_errno_internal(int n) {
    errno = n;
    return -1;
}

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

/*  Read pseudo files in paths like /proc /sys
 *  *buf_ptr can be existing dynamic memory or nullptr (if so, this function
 *  will alloc memory automatically).
 *  remember to free the *buf_ptr because in no cases will *buf_ptr be
 *  freed inside this function
 *  return -1 on error, or non-negative value on success
 * */
int read_pseudo_file_at(const char *path, char **buf_ptr, size_t *buf_size_ptr,
                        bool use_customized_syscalls) {
    if (!path || !*path || !buf_ptr || !buf_size_ptr) {
        errno = EINVAL;
        return -1;
    }

    char *buf;
    size_t buf_size, total_read_size = 0;

    /* Existing dynamic buffer, or a new buffer? */
    buf_size = *buf_size_ptr;
    if (!buf_size)
        *buf_ptr = nullptr;
    buf = *buf_ptr;

    /* Open pseudo file */
    int fd = use_customized_syscalls ?
             my_openat(AT_FDCWD, MAPS_FILE, O_RDONLY | O_CLOEXEC | O_NOCTTY, 0)
                                     : openat(AT_FDCWD, MAPS_FILE, O_RDONLY | O_CLOEXEC | O_NOCTTY,
                                              0);

    if (fd == -1) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "openat error %s : %d", strerror(errno), errno);
        return -1;
    }

    while (true) {
        if (total_read_size >= buf_size) {
            /* linear size growth
             * buf_size grow ~4k bytes each time, 32 bytes for zero padding
             * */
            buf_size = (total_read_size | 4095) + 4097 - 32;
            buf = (char *) realloc(buf, buf_size);
            if (!buf) {
                close(fd);
                errno = ENOMEM;
                return -1;
            }
            *buf_ptr = buf;
            *buf_size_ptr = buf_size;
        }

        size_t n = use_customized_syscalls ?
                   my_read(fd, buf + total_read_size, buf_size - total_read_size)
                                           : read(fd, buf + total_read_size,
                                                  buf_size - total_read_size);
        if (n > 0) {
            total_read_size += n;
        } else if (n == 0) {
            break;
        } else if (n == -1) {
            const int saved_errno = errno;
            close(fd);
            errno = saved_errno;
            return -1;
        }
    }

    if (close(fd) == -1) {
        /* errno set by close(). */
        return -1;
    }

    if (total_read_size + 32 > buf_size)
        memset(buf + total_read_size, 0, 32);
    else
        memset(buf + total_read_size, 0, buf_size - total_read_size);

    errno = 0;
    return total_read_size;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_xxr0ss_antifrida_utils_AntiFridaUtil_nativeGetProcMaps(JNIEnv *env, jobject thiz,
                                                                jboolean useCustomizedSyscall) {
    char *data = nullptr;
    size_t data_size = 0;

    int res = read_pseudo_file_at(MAPS_FILE, &data, &data_size, useCustomizedSyscall);
    if (res == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG,
                            "read_pseudo_file %s failed, errno %s: %d",
                            MAPS_FILE, strerror(errno), errno);
        if (data) {
            free(data);
        }
        return nullptr;
    } else if (res == 0) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "read_pseudo_file had read 0 bytes");
        if (data) {
            free(data);
        }
        return nullptr;
    }
    jstring str = env->NewStringUTF(data);
    free(data);
    return str;
}
