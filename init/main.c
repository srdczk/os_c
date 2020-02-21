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
    console_print("\n");
    show_memory();
}
