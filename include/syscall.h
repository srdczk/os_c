//
// Created by srdczk on 20-3-4.
//
#pragma once

#include "types.h"
#include "idt.h"

#define SYS_GETPID 0
#define SYS_WRITE 1
// 最多 32 个 系统调用
#define SYSCALL_SIZE 0x20

// 无参数的系统调用
#define _syscall0(NUM) ({ \
    int retval;           \
    asm volatile (        \
            "int $0x80"   \
            : "=a"(retval)\
            : "a"(NUM)    \
            : "memory"    \
            );            \
    retval;               \
})

// 一个参数的系统调用
 #define _syscall1(NUM, ARG1) ({ \
     int retval;           \
     asm volatile (        \
             "int $0x80"   \
             : "=a"(retval)\
             : "a"(NUM), "b"(ARG1)    \
             : "memory"    \
             );            \
     retval;               \
 })



u32 sys_getpid(u32 *arg);

u32 getpid();

u32 sys_write(u32 *arg);

u32 write(char *str);

void syscall(int_frame *tf);

