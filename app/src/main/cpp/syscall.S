#include "bionic_asm.h"

#if defined(__aarch64__)

ENTRY(my_read)
    mov     x8, __NR_read
    svc     #0
    cmn     x0, #(MAX_ERRNO + 1)
    cneg    x0, x0, hi
    b.hi    __set_errno_internal
    ret
END(my_read)

ENTRY(my_openat)
    mov     x8, __NR_openat
    svc     #0
    cmn     x0, #(MAX_ERRNO + 1)
    cneg    x0, x0, hi
    b.hi    __set_errno_internal
    ret
END(my_openat)

ENTRY(my_ptrace)
    mov     x8, __NR_ptrace
    svc     #0
    cmn     x0, #(MAX_ERRNO + 1)
    cneg    x0, x0, hi
    b.hi    __set_errno_internal
    ret
END(my_ptrace)
# endif

#if defined(__arm__)
ENTRY(my_read)
    mov     ip, sp
    stmfd   sp!, {r4, r5, r6, r7}
    .cfi_def_cfa_offset 16
    .cfi_rel_offset r4, 0
    .cfi_rel_offset r5, 4
    .cfi_rel_offset r6, 8
    .cfi_rel_offset r7, 12
    ldmfd   ip, {r4, r5, r6}
    ldr     r7, =__NR_read
    swi     #0
    ldmfd   sp!, {r4, r5, r6, r7}
    .cfi_def_cfa_offset 0
    cmn     r0, #(MAX_ERRNO + 1)
    bxls    lr
    neg     r0, r0
    b       __set_errno_internal
END(my_read)

ENTRY(my_openat)
    mov     ip, sp
    stmfd   sp!, {r4, r5, r6, r7}
    .cfi_def_cfa_offset 16
    .cfi_rel_offset r4, 0
    .cfi_rel_offset r5, 4
    .cfi_rel_offset r6, 8
    .cfi_rel_offset r7, 12
    ldmfd   ip, {r4, r5, r6}
    ldr     r7, =__NR_openat
    swi     #0
    ldmfd   sp!, {r4, r5, r6, r7}
    .cfi_def_cfa_offset 0
    cmn     r0, #(MAX_ERRNO + 1)
    bxls    lr
    neg     r0, r0
    b       __set_errno_internal
END(my_openat)

ENTRY(my_ptrace)
    mov     ip, r7
    mov     ip, sp
    stmfd   sp!, {r4, r5, r6, r7}
    .cfi_def_cfa_offset 16
    .cfi_rel_offset r4, 0
    .cfi_rel_offset r5, 4
    .cfi_rel_offset r6, 8
    .cfi_rel_offset r7, 12
    ldmfd   ip, {r4, r5, r6}
    ldr     r7, =__NR_ptrace
    swi     #0
    ldmfd   sp!, {r4, r5, r6, r7}
    .cfi_def_cfa_offset 0
    cmn     r0, #(MAX_ERRNO + 1)
    bxls    lr
    neg     r0, r0
    b       __set_errno_internal
END(my_ptrace)
#endif

#if defined(__x86_64__)
ENTRY(my_read)
    movl    $__NR_read, %eax
    syscall
    cmpq    $-MAX_ERRNO, %rax
    jb      my_read_return
    negl    %eax
    movl    %eax, %edi
    call    __set_errno_internal
my_read_return:
    ret
END(my_read)

ENTRY(my_openat)
    movl    $__NR_openat, %eax
    syscall
    cmpq    $-MAX_ERRNO, %rax
    jb      my_openat_return
    negl    %eax
    movl    %eax, %edi
    call    __set_errno_internal
my_openat_return:
    ret
END(my_openat)

ENTRY(my_ptrace)
    movl    $__NR_ptrace, %eax
    syscall
    cmpq    $-MAX_ERRNO, %rax
    jb      my_ptrace_return
    negl    %eax
    movl    %eax, %edi
    call    __set_errno_internal
my_ptrace_return:
    ret
END(my_ptrace)

#endif