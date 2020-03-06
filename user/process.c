//
// Created by srdczk on 20-3-4.
//
#include "../include/process.h"
#include "../include/x86.h"
#include "../include/console.h"
#include "../include/idt.h"
#include "../include/gdt.h"
#include "../include/pmm.h"
#include "../include/thread.h"

#define USER_STACK_TOP (0xc0000000 - 0x1000)

void print() {
    int i = 0;
    while (1) {
        if (i % 2) {
            i = 0;
            console_print_color("SI", GREEN);
            console_print("\n");
        } else {
            i = 1;
            console_print_color("SIMA", RED);
            console_print("\n");
        }
    }
}

void switch_to_user() {
    // 测试, 在用户态 运行程序( 特权级 为 3 )
    u32 *esp = (u32 *)kmalloc_page(1, kernel_pde);
    esp = (u32 *)((u32) esp + PAGE_SIZE);
    update_esp0((u32) esp);
    *(--esp)  = USER_DS;
    *(--esp) = 0xc0400000;
    *(--esp) = EFLAGS_IF_1 | EFLAGS_MBS | EFLAGS_IOPL_0;
    *(--esp) = USER_CS;
    *(--esp) = (u32) print;

    asm volatile ("movl %0, %%esp;"
                  "mov $0x23, %%eax;"
                  "mov %%ax, %%gs;"
                  "mov %%ax, %%fs;"
                  "mov %%ax, %%ds;"
                  "mov %%ax, %%es;"
                  "iret" :: "g"((u32) esp) : "memory");
}

void start_process(void *func) {
    // 获取当前 运行进程 的栈底
    task_struct *pthread = running_thread();
    u32 *esp = (u32 *)((u32) pthread + PAGE_SIZE);
    *(--esp)  = USER_DS;
    // 先设置用户栈底 为用户空间申请的0xc0000000 - 0x1000 + 0x1000, 先只调度一个用户进程
    *(--esp) = ((u32)(sync_get_user_page(USER_STACK_TOP)) + 0x1000);
    *(--esp) = EFLAGS_IF_1 | EFLAGS_MBS | EFLAGS_IOPL_0;
    *(--esp) = USER_CS;
    *(--esp) = (u32) func;

    asm volatile ("cli;"
                  "movl %0, %%esp;"
                  "mov $0x23, %%eax;"
                  "mov %%ax, %%gs;"
                  "mov %%ax, %%fs;"
                  "mov %%ax, %%ds;"
                  "mov %%ax, %%es;"
                  "iret" :: "g"((u32) esp) : "memory");
    // 第一次运行, 进入用户态
}

void pde_enable(task_struct *pthread) {
    u32 pde_addr;
    if (!pthread->pgdir) pde_addr = (u32) kernel_pde - KERNEL_PAGE_OFFSET;
    else pde_addr = va2pa(pthread->pgdir);
    // 启用特定页表
    asm volatile("mov %0, %%cr3" :: "r"(pde_addr));
}

void process_enable(task_struct *pthread) {
    pde_enable(pthread);

    if (pthread->pgdir) update_esp0((u32) pthread + PAGE_SIZE);

}

void user_pool_init(task_struct *thread) {
    // 初始化用户位图
    thread->user_pool.addr_start = USER_OFFSET;
    u32 page_cnt = DIV_ROUND_UP((KERNEL_PAGE_OFFSET - USER_OFFSET) / (PAGE_SIZE * 8), PAGE_SIZE);
    thread->user_pool.bmap.map = kmalloc_page(page_cnt, kernel_pde);
    thread->user_pool.bmap.map_len = (KERNEL_PAGE_OFFSET - USER_OFFSET) / (PAGE_SIZE * 8);
    bitmap_init(&thread->user_pool.bmap);
}

// 创建进程自己的页表
void *create_pde() {
    // 内核空间申请
    u32 *pde = kmalloc_page(1, kernel_pde);
    // 拷贝内核页目录的项
    int i;
    for (i = 0; i < 1024; ++i) pde[i] = kernel_pde[i];
    // 获取页表真实物理地址
    u32 phy_addr = va2pa((u32) pde);

    pde[1023] = phy_addr | PG_USER | PG_RW | PG_PRESENT;

    return (void *) pde;
}

void process_exec(void *func, char *name) {
    task_struct *thread = kmalloc_page(1, kernel_pde);
    thread_init(thread, name, DEFAULT_PRIO);
    user_pool_init(thread);
    thread_create(thread, start_process, func);
    thread->pgdir = (u32) create_pde();
    // 初始化 进程 arena 描述符号
    area_desc_init(thread->user_arena);
    list_add_last(&ready_list, &thread->general_tag);
    list_add_last(&global_list, &thread->global_tag);
}

