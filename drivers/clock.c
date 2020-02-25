#include "../include/clock.h"
#include "../include/x86.h"
u32 ticks;

void clock_init(u32 fre) {
    u32 divisor = 1193180 / fre;

	outb(0x43, 0x36);

	u8 low = (u8)(divisor & 0xff);
	u8 hign = (u8)((divisor >> 8) & 0xff);

	outb(0x40, low);
	outb(0x40, hign);
    ticks = 0;
}

