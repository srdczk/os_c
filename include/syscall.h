//
// Created by srdczk on 20-3-4.
//
#pragma once

#include "types.h"
#include "idt.h"

#define SYS_GETPID 0

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

void syscall(int_frame *tf);