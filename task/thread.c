#include "../include/thread.h"
#include "../include/string.h"
#include "../include/debug.h"
#include "../include/idt.h"
#include "../include/isr.h"
#include "../include/pmm.h"
#include "../include/process.h"

#define TSTACK_SIZE 0x1000

int global_pid = 0;

task_struct *cur_thread;
// 就绪队列和全局队列
list ready_list;
list global_list;


static void kernel_thread(thread_func *func, void *arg) {
    // 打开中断
    enable_int();
    func(arg);
}

// 汇编实现 
extern void switch_to(task_struct *cur, task_struct *next);

task_struct *running_thread() {
    // 获得当前运行的 pcb, 都是内核态 运行的
    u32 esp;
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    return (task_struct *)((u32)(esp & 0xfffff000));
}

void thread_create(task_struct *pthread, thread_func *func, void *arg) {
    // 对于用户进程, 顶部存储 ss, eip, eflags, cs, eip 即可
    pthread->self_stack -= sizeof(u32) * 5;
    pthread->self_stack -= sizeof(task_stack);

    task_stack *thread_stack = (task_stack *)pthread->self_stack;
    thread_stack->ebx = thread_stack->ebp = thread_stack->edi = thread_stack->esi = 0;

    thread_stack->ret_func = (u32)kernel_thread;
    thread_stack->func = func;
    thread_stack->arg = arg;

}

void thread_init(task_struct *pthread, char *name, u32 priority) {
    memset((char *)pthread, '\0', sizeof(task_struct));
    strcpy(name, pthread->name);
    if (cur_thread == pthread) pthread->status = RUNNING;
    else pthread->status = READY;
    // 和正在运行的内核栈 统一 -> 分配 1 页 大小的栈空间
    pthread->self_stack = (u32)pthread + TSTACK_SIZE;
    pthread->priority = priority;
    pthread->ticks = priority;
    pthread->running_ticks = 0;
    pthread->pgdir = 0;
    pthread->stack_magic = 0xabbacddc;
}

task_struct *thread_start(char *name, int priority, thread_func *func, void *arg) {
    task_struct *pthread = kmalloc_page(1, kernel_pde);
    thread_init(pthread, name, (u32) priority);
    thread_create(pthread, func, arg);

    list_add_last(&ready_list, &pthread->general_tag);

    list_add_last(&global_list, &pthread->global_tag);
    return pthread;
}

static void main_thread_init() {
    // 开始的时候, 正在运行
    cur_thread = running_thread();
    // 正在运行的线程
    thread_init(cur_thread, "main", 31);
    // 加进全局队列中
    list_add_last(&global_list, &cur_thread->global_tag);
}

void kernel_thread_init() {
    list_init(&ready_list);
    list_init(&global_list);
    main_thread_init();
}

void schedule() {
    // 加入 ready list, 并从 ready list 中弹出 最前的运行 switch_to
    task_struct *now_thread = cur_thread;
    ++now_thread->running_ticks;
    if (!--now_thread->ticks) {
        // 开始进行调度
        now_thread->ticks = now_thread->priority;
        now_thread->status = READY;
        list_add_last(&ready_list, &now_thread->general_tag);
        list_node *ready = list_pop_first(&ready_list);
        task_struct *next = node2entry(task_struct, general_tag, ready);
        cur_thread = next;
        cur_thread->status = RUNNING;
        process_enable(next);
        switch_to(now_thread, next);
    }
}

void thread_block(task_status status) {
    ASSERT(status == BLOCKED || status == HANGING || status == WAITING);
    disable_int();
    task_struct *now_thread = cur_thread;
    now_thread->status = status;
    list_node *ready = list_pop_first(&ready_list);
    task_struct *next = node2entry(task_struct, general_tag, ready);
    cur_thread = next;
    next->status = RUNNING;
    process_enable(next);
    switch_to(now_thread, next);
    enable_int();
}

void thread_unblock(task_struct *pthread) {
    // 将其加入到 ready 队列最前
    disable_int();
    ASSERT(!find_node(&ready_list, &pthread->general_tag));
    pthread->status = READY;
    list_add_first(&ready_list, &pthread->general_tag);
    enable_int();
}
