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
    //clock_init(200);
    //sti();
    console_print("BEGIN: 0x");
    console_print_hex((u32)kern_start, RED);
    console_print(", END: 0x");
    console_print_hex((u32)kern_end, RED);
    console_print(", Sum: ");
    u32 size = ((u32)kern_end - (u32)kern_start) / 1024;
    console_print_dec(size, GREEN);
    console_print("\n");
    show_memory();
    pmm_init();
    console_print("Page: ");
    console_print_dec(pmm_stack_size, RED);
    console_print("\n");
    console_print_hex(alloc_page(), GREEN);
}
