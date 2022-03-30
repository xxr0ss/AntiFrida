#include "bionic_asm.h"

#if defined(__aarch64__)

ENTRY(my_read)
    mov     x8, __NR_read
    svc     #0

    cmn     x0, #(MAX_ERRNO + 1)
    cneg    x0, x0, hi

    // we do not call __set_errno_internal due to a
    // bug related to https://github.com/android/ndk/issues/1422
    // TODO: if solutions found, fix antifrida.cpp
    // b.hi    __set_errno_internal

    ret
END(my_read)

#elif defined(__x86_64__)

ENTRY(my_read)
    movl    $__NR_read, %eax
    syscall
    cmpq    $-MAX_ERRNO, %rax
    jb      return
    negl    %eax
    movl    %eax, %edi
    // call    __set_errno_internal
return:
    ret
END(my_read)

#endif