//
// Created by srdczk on 20-3-4.
//
#include "../include/syscall.h"
#include "../include/thread.h"
#include "../include/console.h"
#include "../include/debug.h"
#include "../include/string.h"

static u32 (*syscalls[])(u32 *arg) = {
    sys_getpid,
    sys_write,
    sys_malloc,
    sys_free
};


u32 sys_getpid(u32 *arg) {
    return running_thread()->pid;
}

u32 getpid() {
    return (u32) _syscall0(SYS_GETPID);
}

u32 sys_write(u32 *arg) {
    char *str = arg[0];
    console_print(str);
    return strlen(str);
}

u32 write(char *str) {
    return (u32) _syscall1(SYS_WRITE, str);
}

u32 sys_malloc(u32 *arg) {
    u32 size = (u32) arg[0];
    return (u32) pmm_malloc(size);
}

void *malloc(u32 size) {
    return (void *) _syscall1(SYS_MALLOC, size);
}

u32 sys_free(u32 *arg) {
    void *ptr = (void *)arg[0];
    pmm_free(ptr);
    return 0;
}

void free(void *ptr) {
    _syscall1(SYS_FREE, ptr);
}

void syscall(int_frame *tf) {
    u32 arg[5];
    int num = tf->eax;
    if (num >= 0 && num < SYSCALL_SIZE) {
        if (syscalls[num] != NULL) {
            arg[0] = tf->edx;
            arg[1] = tf->ecx;
            arg[2] = tf->ebx;
            arg[3] = tf->edi;
            arg[4] = tf->esi;
            tf->eax = syscalls[num](arg);
            return ;
        }
    }
    PANIC("WRONG SYSCALL NUMBER!");
}

