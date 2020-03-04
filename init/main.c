#include "../include/console.h"
#include "../include/keyboard.h"
#include "../include/sync.h"
#include "../include/pmm.h"
#include "../include/list.h"
#include "../include/debug.h"
#include "../include/clock.h"
#include "../include/isr.h"
#include "../include/gdt.h"
#include "../include/process.h"
#include "../include/thread.h"
// 设置临时页表

#define STACK_SIZE 0x1000

void kernel_init();

multiboot *global_multiboot;

char kernel_stack[STACK_SIZE] __attribute__((aligned(STACK_SIZE)));

__attribute__((section(".init.data"))) u32 *tmp_pde = (u32 *)0x1000;
__attribute__((section(".init.data"))) u32 *low_pte = (u32 *)0x2000;
__attribute__((section(".init.data"))) u32 *high_pte = (u32 *)0x3000;

__attribute__((section(".init.text"))) void main() {
    tmp_pde[0] = (u32)low_pte | PG_RW | PG_PRESENT;
    tmp_pde[(0xc0000000 >> 22) & 0x3ff] = (u32)high_pte | PG_RW | PG_PRESENT;

    int i;
    for (i = 0; i < 1024; ++i) {
        low_pte[i] = high_pte[i] = (u32) ((i << 12) | PG_RW | PG_PRESENT);
    }

    asm volatile ("mov %0, %%cr3" :: "r"(tmp_pde));
    // 开启分页
    u32 cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));
    // 新的栈顶
    u32 kernel_stop = ((u32)kernel_stack + STACK_SIZE);
    asm volatile ("mov %0, %%esp\n\t"
			"xor %%ebp, %%ebp" : : "r" (kernel_stop));
    global_multiboot = tmp_multiboot + 0xc0000000;
    kernel_init();
}

// 键盘缓冲区 消费者
void thread(void *arg) {
    while (1) {
        buffer_getchar(&kb_buffer);
    }
}

void thread_a(void *arg) {
    while (1) {
        console_print("THREADA");
        console_print("\n");
    }
}

void thread_b(void *arg) {
    while (1) {
        console_print_color("THREADB", MAGENTA);
        console_print("\n");
    }
}

void u_proc_a() {
    while (1) {
        console_print_color("USERA", GREEN);
        console_print("\n");
    }
}

void u_proc_b() {
    while (1) {
        console_print_color("USERB", RED);
        console_print("\n");
    }
}


void kernel_init() {
    console_clear();
    gdt_init();
    clock_init(1000);
    idt_init();
    pmm_init();
    kernel_thread_init();
    thread_start("THREADA", 23, thread_a, NULL);
    thread_start("THREADB", 23, thread_b, NULL);
    process_exec(u_proc_a, "PROCA");
    process_exec(u_proc_b, "PROCB");
    enable_int();
    while (1) {
        asm volatile ("hlt");
    }
}


