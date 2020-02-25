#include "../include/console.h"
#include "../include/pmm.h"
#include "../include/clock.h"
#include "../include/isr.h"
#include "../include/gdt.h"
#include "../include/heap.h"
// 设置临时页表

#define STACK_SIZE 8192

void kernel_init();

multiboot *global_multiboot;

char kernel_stack[STACK_SIZE];

__attribute__((section(".init.data"))) u32 *tmp_pde = (u32 *)0x1000;
__attribute__((section(".init.data"))) u32 *low_pte = (u32 *)0x2000;
//__attribute__((section(".init.data"))) u32 *low_pte = (u32 *)0x3000;
__attribute__((section(".init.data"))) u32 *high_pte = (u32 *)0x3000;
//__attribute__((section(".init.data"))) u32 *high_pte_b = (u32 *)0x5000;

__attribute__((section(".init.text"))) void main() {
    // 建立 临时 页表 -> 将 0 - 4M 和 3G - 4M 全部映射 
    tmp_pde[0] = (u32)low_pte | 0x1 | 0x2;
    //tmp_pde[1] = (u32)low_pte_b | 0x1 | 0x2;
    tmp_pde[(0xc0000000 >> 22) & 0x3ff] = (u32)high_pte | 0x1 | 0x2;
    //tmp_pde[(0xc0400000 >> 22) & 0x3ff] = (u32)high_pte_b | 0x1 | 0x2;

    int i;
    for (i = 0; i < 1024; ++i) {
        low_pte[i] = high_pte[i] = (i << 12) | 0x1 | 0x2;
    }
//    for (i = 1024; i < 2048; ++i) {
//        low_pte_b[i] = (i << 12) | 0x1 | 0x2;
//    }
    asm volatile ("mov %0, %%cr3" :: "r"(tmp_pde));
    // 开启分页
    u32 cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));

    u32 kernel_stop = ((u32)kernel_stack + STACK_SIZE) & 0xfffffff0;
    asm volatile ("mov %0, %%esp\n\t"
			"xor %%ebp, %%ebp" : : "r" (kernel_stop));
    global_multiboot = tmp_multiboot + 0xc0000000;
    kernel_init();
}

void kernel_init() {
    console_clear();
    gdt_init();
//    clock_init(200);
    idt_init();
 //   sti();

    show_memory();
    console_print("\n");
    pmm_init();
    heap_init();
    int *a = (int *)kmalloc(4);
    console_print_hex((u32)a, RED);
    int *b = (int *)kmalloc(4);
    console_print("\n");
    console_print_hex((u32)b, GREEN);
    console_print("\n");
    kfree(b);
    kfree(a);
    a = (int *)kmalloc(12);
    console_print_hex((u32)a, GREEN);
    console_print_dec(physical_page_cnt, RED);
    console_print("\n");
    console_print_dec(((u32)kernel_end - (u32)kernel_start) / 1024, GREEN);
    while (1) {
        asm volatile ("hlt");
    }
}
