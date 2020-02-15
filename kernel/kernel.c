#include "../trap/isr.h"
void main() {
    clear_screen();
    irq_init();
    idt_init();
    clock_init();
    sti();
//    while (1);
}
