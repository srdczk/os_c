#include "irq.h"

#define IO_PIC1 0x20 // IRQ 0 - 7
#define IO_PIC2 0xa0 // IRQ 8 - 15

#define IRQ_SLAVE 2

u16 irq_mask = 0xffff & ~(1 << IRQ_SLAVE);
u8 did_init = 0;

void irq_setmask(u16 mask) {
    irq_mask = mask;
    if (did_init) {
        outb(IO_PIC1, mask);
        outb(IO_PIC2, mask >> 8);
    }
}

void irq_enable(u32 irq) {
    irq_setmask(irq_mask & ~(1 << irq));
}

void irq_init() {
    did_init = 1;
    outb(IO_PIC1 + 1, 0xff);
    outb(IO_PIC2 + 1, 0xff);
    outb(IO_PIC1, 0x11);
    outb(IO_PIC1 + 1, IRQ_OFFSET);
    outb(IO_PIC1 + 1, 1 << IRQ_SLAVE);
    outb(IO_PIC1 + 1, 0x3);
    outb(IO_PIC2, 0x11);    // ICW1
    outb(IO_PIC2 + 1, IRQ_OFFSET + 8);  // ICW2
    outb(IO_PIC2 + 1, IRQ_SLAVE);       // ICW3
    outb(IO_PIC2 + 1, 0x3);             // ICW4

    outb(IO_PIC1, 0x68);    // clear specific mask
    outb(IO_PIC1, 0x0a);    // read IRR by default

    outb(IO_PIC2, 0x68);    // OCW3
    outb(IO_PIC2, 0x0a);    // OCW3

    if (irq_mask != 0xFFFF) {
        irq_setmask(irq_mask);
    }
}
