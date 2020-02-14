#include "idt.h"

void set_idt_gate(int n, u32 handler) {
    idt[n].low_offset = handler & 0xffff;
    idt[n].selector = KERNEL_CS;
    idt[n].always0 = 0;
    idt[n].flags = 0x8e;
    idt[n].high_offset = (handler >> 16) & 0xffff;
}

void set_idt() {
    idt_desc.base = (u32)&idt;
    idt_desc.limit = IDT_SIZE * sizeof(idt_gate) - 1;
    lidt(&idt_desc);
}
