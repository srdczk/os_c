#include "../trap/isr.h"
#include "../mm/pmm.h"
void main() {
    clear_screen();
    irq_init();
    idt_init();
    clock_init();
    sti();
    page_init();

    int *c = (int *)0xa0000000;
    u32 do_page_ptr = *c;
//    while (1);
}
