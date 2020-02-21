#include "../include/console.h"
#include "../include/clock.h"
#include "../include/gdt.h"
#include "../include/x86.h"
#include "../include/isr.h"
int main() {
    console_clear();
    gdt_init();
    idt_init();
    clock_init(200);
    sti();
}
