#include "clock.h"

u32 ticks;

void clock_init() {
    outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
    outb(IO_TIMER1, TIMER_DIV(100) % 256);
    outb(IO_TIMER1, TIMER_DIV(100) / 256);
    ticks = 0;
    kprint("++ setup timer !");
    kprint("\n");
    irq_enable(IRQ_TIMER);
}
