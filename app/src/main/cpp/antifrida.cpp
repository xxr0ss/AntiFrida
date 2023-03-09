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
#include <elf.h>
#include <link.h>
#include <sys/ptrace.h>


#define unused_param(x) (x)

const char MAPS_FILE[] = "/proc/self/maps";
const char TAG[] = "JNI";

// customized syscalls
extern "C" int my_read(int, void *, size_t);
extern "C" int
my_openat(int dirfd, const char *const __pass_object_size pathname, int flags, mode_t modes);
extern "C" long my_ptrace(int __request, ...);

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
    int fd = use_customized_syscalls
             ? my_openat(AT_FDCWD, path, O_RDONLY | O_CLOEXEC, 0)
             : openat(AT_FDCWD, path, O_RDONLY | O_CLOEXEC, 0);

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

        ssize_t n = use_customized_syscalls
                   ? my_read(fd, buf + total_read_size, buf_size - total_read_size)
                   : read(fd, buf + total_read_size, buf_size - total_read_size);
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
    return (int)total_read_size;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_xxr0ss_antifrida_utils_AntiFridaUtil_nativeReadProcMaps(JNIEnv *env, jobject thiz,
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

int read_line(int fd, char *ptr, unsigned int maxlen, jboolean use_customized_syscall) {
    int n;
    long rc;
    char c;

    for (n = 1; n < maxlen; n++) {
        rc = use_customized_syscall ? my_read(fd, &c, 1) : read(fd, &c, 1);
        if (rc == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return 0;    /* EOF no data read */
            else
                break;    /* EOF, some data read */
        } else
            return (-1);    /* error */
    }
    *ptr = 0;
    return (n);
}

int wrap_endsWith(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenA = strlen(str);
    size_t lenB = strlen(suffix);
    if (lenB > lenA)
        return 0;
    return strncmp(str + lenA - lenB, suffix, lenB) == 0;
}

int elf_check_header(uintptr_t base_addr) {
    auto *ehdr = (ElfW(Ehdr) *) base_addr;
    if (0 != memcmp(ehdr->e_ident, ELFMAG, SELFMAG)) return 0;
#if defined(__LP64__)
    if (ELFCLASS64 != ehdr->e_ident[EI_CLASS]) return 0;
#else
    if (ELFCLASS32 != ehdr->e_ident[EI_CLASS]) return 0;
#endif
    if (ELFDATA2LSB != ehdr->e_ident[EI_DATA]) return 0;
    if (EV_CURRENT != ehdr->e_ident[EI_VERSION]) return 0;
    if (ET_EXEC != ehdr->e_type && ET_DYN != ehdr->e_type) return 0;
    if (EV_CURRENT != ehdr->e_version) return 0;
    return 1;
}

int find_mem_string(uint64_t base, uint64_t end, unsigned char *ptr, unsigned int len) {
    auto *rc = (unsigned char *) base;

    while ((uint64_t) rc < end - len) {
        if (*rc == *ptr) {
            if (memcmp(rc, ptr, len) == 0) {
                return 1;
            }
        }
        rc++;
    }
    return 0;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_xxr0ss_antifrida_utils_AntiFridaUtil_scanModulesForSignature(JNIEnv *env, jobject thiz,
                                                                      jstring signature,
                                                                      jboolean use_customized_syscalls) {
    int fd = use_customized_syscalls ? my_openat(AT_FDCWD, MAPS_FILE, O_RDONLY | O_CLOEXEC, 0)
                                     : openat(AT_FDCWD, MAPS_FILE, O_RDONLY | O_CLOEXEC, 0);
    if (fd == -1) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "openat error %s : %d", strerror(errno), errno);
        return -1;
    }

    const int buf_size = 512;
    char buf[buf_size];
    char path[256];
    char perm[5];
    const char *sig = env->GetStringUTFChars(signature, nullptr);
    size_t sig_len = strlen(sig);

    uint64_t base, end, offset;
    jboolean result = JNI_FALSE;

    while ((read_line(fd, buf, buf_size, use_customized_syscalls)) > 0) {
        if (sscanf(buf, "%lx-%lx %4s %lx %*s %*s %s", &base, &end, perm, &offset, path) != 5) {
            continue;
        }

        if (perm[0] != 'r') continue;
        if (perm[3] != 'p') continue; //do not touch the shared memory
        if (0 != offset) continue;
        if (strlen(path) == 0) continue;
        if ('[' == path[0]) continue;
        if (end - base <= 1000000) continue;
        if (wrap_endsWith(path, ".oat")) continue;
        if (elf_check_header(base) != 1) continue;

        if (find_mem_string(base, end, (unsigned char *) sig, sig_len) == 1) {
            __android_log_print(ANDROID_LOG_INFO, TAG,
                                "frida signature \"%s\" found in %lx - %lx", sig, base, end);
            result = JNI_TRUE;
            break;
        }
    }

    return result;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_xxr0ss_antifrida_utils_AntiFridaUtil_checkBeingDebugged(JNIEnv *env, jobject thiz,
                                                                 jboolean use_customized_syscall) {

    long res = use_customized_syscall
               ? my_ptrace(PTRACE_TRACEME, 0)
               : ptrace(PTRACE_TRACEME, 0);
    return res < 0 ? JNI_TRUE : JNI_FALSE;
}