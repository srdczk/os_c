#include "../include/clock.h"
#include "../include/irq.h"
#include "../include/thread.h"
#include "../include/x86.h"
u32 ticks;

#define IRQ_CLOCK 0x20

void clock_init(u32 fre) {
    u32 divisor = 1193180 / fre;

	outb(0x43, 0x36);

	u8 low = (u8)(divisor & 0xff);
	u8 hign = (u8)((divisor >> 8) & 0xff);

	outb(0x40, low);
	outb(0x40, hign);
    ticks = 0;

    // 设置 中断可用
//    irq_enable(IRQ_CLOCK);

}

static void ticks_sleep(u32 tick) {
	u32 start = ticks;
	while (ticks - start < tick) thread_yield();
}

void mil_sleep(u32 m_seconds) {
	u32 sticks = DIV_ROUND_UP(m_seconds, MIL_PER_INT);
	ticks_sleep(sticks);
}

