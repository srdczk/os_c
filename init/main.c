#include "../include/console.h"
#include "../include/keyboard.h"
#include "../include/stdio.h"
#include "../include/sync.h"
#include "../include/pmm.h"
#include "../include/ide.h"
#include "../include/list.h"
#include "../include/debug.h"
#include "../include/clock.h"
#include "../include/syscall.h"
#include "../include/isr.h"
#include "../include/gdt.h"
#include "../include/fs.h"
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

semaphore x, y;

//lock lck;

int p = 0;

void thread_a(void *arg) {
    void *x = pmm_malloc(256);
    void *y = pmm_malloc(255);
    void *z = pmm_malloc(254);
    console_print("AX:"); console_print_hex((u32) x, GREEN);
    console_print("AY:"); console_print_hex((u32) y, MAGENTA);
    console_print("AZ:"); console_print_hex((u32) z, RED);
    int cpu_delay = 100000;
    while (cpu_delay-- > 0);
    pmm_free(x);
    pmm_free(y);
    pmm_free(z);
    console_print_color("NIMASILE\n", WHITE);
    while (1);
}

void thread_b(void *arg) {
    kprintf("NIMASILE TA\n");
    while (1);
}

void u_proc_a() {
    printf("NIMASIL:E%x\n", 0x34);
    while (1);
}

void u_proc_b() {
    void *addr = malloc(63);
    printf("PPP1:0x%x\n", addr);
    free(addr);
    addr = malloc(127);
    printf("PPP1:0x%x\n", addr);
    void *addr2 = malloc(36);
    printf("PPP2:0x%x\n", addr2);
    free(addr);
    free(addr2);
    printf("LAST:0x%x\n", malloc(33));
    while (1);
}


void kernel_init() {
    console_clear();
    gdt_init();
    clock_init(FREQUENCY);
    keyboard_init();
    idt_init();
    pmm_init();
    kernel_thread_init();
    ide_init();
    filesys_init();
    enable_int();
    int fd = fs_open("/file1", O_CREAT | O_RDWR);
    kprintf("\n create res:%d \n", fd);
    int res = fs_write(fd, "hello!\n", 7);
    kprintf("\nwrite res:%d\n", res);
    char buf[10] = {0};
    fs_lseek(fd, 0, SEEK_SET);
    int len = fs_read(fd, buf, 7);
    kprintf("\nread %d res:%s\n",len, buf);
    fs_close(fd);
    thread_start("TA", 15, thread_b, NULL);
    process_exec(u_proc_a, "PA");
    while (1);
}


