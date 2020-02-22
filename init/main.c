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
    int *a = (int *)0xa0000000;
    *a = 3;
}
