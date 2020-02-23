#include "../include/console.h"
#include "../include/clock.h"
#include "../include/gdt.h"
#include "../include/x86.h"
#include "../include/isr.h"
#include "../include/pmm.h"
int main() {
    console_clear();
    gdt_init();
    idt_init();
    pmm_init();
    //clock_init(200);
    //sti();
    //int *a = (int *)0xa0000000;
    //*a = 3;
    show_memory();
    console_print("\n");
    console_print_dec(((u32)kernel_end - (u32)kernel_start) / 1024, GREEN);
    console_print_dec(pmm_stack_size, RED);
}
