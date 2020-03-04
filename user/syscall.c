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
    sys_write
};


u32 sys_getpid(u32 *arg) {
    return running_thread()->pid;
}

u32 getpid() {
    return _syscall0(SYS_GETPID);
}

u32 sys_write(u32 *arg) {
    char *str = arg[0];
    console_print(str);
    return strlen(str);
}

u32 write(char *str) {
    return _syscall1(SYS_WRITE, str);
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

