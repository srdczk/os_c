#include "../include/idt.h"

#define KERNEL_CS 0x08
#define IDT_SIZE 256

idt_gate idt[IDT_SIZE];
descriptor idt_desc;

void set_idt_gate(u32 num, u32 handler) {
    idt[num].low_offset = handler & 0xffff;
    idt[num].selector = KERNEL_CS;
    idt[num].always0 = 0;
    idt[num].flags = 0x8e;
    idt[num].high_offset = (handler >> 16) & 0xffff;
}

void set_idt() {
    idt_desc.base = (u32)&idt;
    idt_desc.limit = IDT_SIZE * sizeof(idt_gate) - 1;
    lidt(&idt_desc);
}

