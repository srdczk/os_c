#include "../include/thread.h"
#include "../include/pmm.h"
#include "../include/string.h"
#include "../include/idt.h"
#include "../include/console.h"
#include "../include/schedule.h"
u32 global_pid = 0;

u32 kernel_thread(thread_func *func, void *arg) {
    task_struct *new_task = malloc_page(0, 2);
    // 栈低端设置为 0
    memset((char *)new_task, '\0', sizeof(task_struct));
    new_task->state = RUNNABLE;
    new_task->stack = cur;
    new_task->pid = global_pid++;
    u32 *stack_top = (u32 *)((u32)new_task + 2 * PAGE_SIZE);
    *(--stack_top) = (u32)arg;
    *(--stack_top) = (u32)kernel_thread_exit;
    *(--stack_top) = (u32)func;
    new_task->text.esp = (u32)new_task + 2 * PAGE_SIZE - sizeof(u32) * 3;
    // 中断设置为 开
    new_task->text.eflags |= 0x200;
    new_task->next = running_head;
    task_struct *tail = running_head;
    while (tail->next != running_head) tail = tail->next;
    tail->next = new_task;
    return new_task->pid;
}

void kernel_thread_exit() {
    register uint32_t val asm ("eax");

    console_print("EXIT\n");

    while (1);
}
