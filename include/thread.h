#pragma once

#include "types.h"
#include "arena.h"
#include "list.h"
#include "pmm.h"

typedef void thread_func(void *);

typedef enum {
    RUNNING,
    BLOCKED,
    WAITING,
    HANGING,
    READY,
    DIED
} task_status;

// kernel_thread 参数 -> kernel_thread(thread_func *, void *)
typedef struct {
    // caller 保存寄存器
    u32 ebp, ebx, edi, esi;
    u32 ret_func;
    u32 unused_ret;
    // 第一个参数
    thread_func *func;
    void *arg;
} task_stack;

// 任务控制块 TCB PCB
typedef struct {
    // 指针 -> esp 指向
    u32 self_stack;
    // 任务状态
    task_status status;
    u32 pid;
    char name[16];
    u32 priority;
    u32 ticks;  // 每次处理器上执行的tick -> 和线程优先级 有关
    // 此任务执行了多久
    u32 running_ticks;
    // 所有 节点中的tag 和一般队列中的tag
    // 用于 转换成 thread
    list_node general_tag;
    list_node global_tag;
    u32 pgdir; // 进程持有, 自己的页表(虚拟地址)
    // 用户进程虚拟内存的bitmap, 用于 malloc
    pool user_pool;
    // 用户进程arena 描述 -> 用于 malloc
    arena_desc user_arena[DESC_NUM];
    // 魔数, 检测栈是否溢出
    u32 stack_magic;
} task_struct;

extern list ready_list;
extern list global_list;

task_struct *running_thread();

void thread_create(task_struct *pthread, thread_func *func, void *arg);

void thread_init(task_struct *pthread, char *name, u32 priority);

task_struct *thread_start(char *name, int priority, thread_func *func, void *arg);

void kernel_thread_init();

void schedule();

void thread_block(task_status status);

void thread_unblock(task_struct *pthread);

