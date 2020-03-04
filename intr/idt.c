#include "../include/x86.h"
#include "../include/idt.h"

//#define KERNEL_CS 0x08
#define IDT_SIZE 256

idt_gate idt[IDT_SIZE];
descriptor idt_desc;

void set_idt_gate(u32 num, u32 handler, u16 sel, u8 flags) {
    idt[num].low_offset = (u16) (handler & 0xffff);
    idt[num].high_offset = (u16) ((handler >> 16) & 0xffff);
    idt[num].selector = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags; // | 0x60 -> 用户态可以设置中断门的特权级别为 3
}

void set_idt() {
    idt_desc.base = (u32)&idt;
    idt_desc.limit = IDT_SIZE * sizeof(idt_gate) - 1;
    lidt(&idt_desc);
}

