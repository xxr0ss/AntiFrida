#include "bionic_asm.h"

ENTRY(my_read)
    mov     x8, __NR_read
    svc     #0

    cmn     x0, #(MAX_ERRNO + 1)
    cneg    x0, x0, hi

    // we do not call __set_errno_internal for a
    // bug related to https://github.com/android/ndk/issues/1422
    // b.hi    __set_errno_internal

    ret
END(my_read)