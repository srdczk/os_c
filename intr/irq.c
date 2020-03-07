//
// Created by srdczk on 20-3-7.
//

#include "../include/irq.h"

#define IO_PIC1             0x20
#define IO_PIC2             0xA0

#define IRQ_OFFSET 0x20

#define IRQ_SLAVE           2       // IRQ at which slave connects to master

static u16 irq_mask = 0xFFFF & ~(1 << IRQ_SLAVE);
static bool did_init = 0;

static void pic_setmask(u16 mask) {
    irq_mask = mask;
    if (did_init) {
        outb(IO_PIC1 + 1, mask);
        outb(IO_PIC2 + 1, mask >> 8);
    }
}

void irq_enable(unsigned int irq) {
    pic_setmask(irq_mask & ~(1 << irq));
}

void irq_init(void) {
    did_init = 1;

    // 初始化主片, 从片
    outb(IO_PIC1 + 1, 0xFF);
    outb(IO_PIC2 + 1, 0xFF);

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
        pic_setmask(irq_mask);
    }
}

