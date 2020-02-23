#include "../include/console.h"
#include "../include/clock.h"
#include "../include/gdt.h"
#include "../include/x86.h"
#include "../include/isr.h"
#include "../include/pmm.h"
#include "../include/heap.h"
int main() {
    console_clear();
    gdt_init();
    idt_init();
    pmm_init();
    heap_init();
    //clock_init(200);
    //sti();
    //int *a = (int *)0xa0000000;
    //*a = 3;
    int *a = (int *)kmalloc(12);
    console_print_color("0x", GREEN);
    console_print_hex((u32)a, GREEN);
    console_print("\n");
    int *b = (int *)kmalloc(4);
    console_print_color("0x", GREEN);
    console_print_hex((u32)b, GREEN);
    console_print("\n");
    kfree(a);
    kfree(b);
    int *c = (int *)kmalloc(5);
    console_print_color("0x", GREEN);
    console_print_hex((u32)c, GREEN);
}
