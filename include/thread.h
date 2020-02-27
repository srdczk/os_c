#pragma once

#include "types.h"

typedef void thread_func(void *);

typedef enum {
    SLEEPING,
    RUNNABLE,
    DIED
} task_state;

// 线程上下文信息
typedef struct {
    // eflags 置为 0x200 -> 开启中断
    u32 esp, ebp, ebx, esi, edi, eflags;
} context;

//任务控制块 tcb / pcb
typedef struct task_struct {
    volatile task_state state;
    u32 pid;
    void *stack; //内核栈
    context text;
    struct task_struct *next; // 链表结构
} task_struct;

// 全局 pid 的值 -> 每次申请 一个pid++

extern u32 global_pid;

u32 kernel_thread(thread_func *func, void *arg);

void kernel_thread_exit();

