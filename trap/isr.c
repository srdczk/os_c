#include "isr.h"

void isr_install() {
    u32 i;
    for (i = 0; i < 32; ++i) {
        set_idt_gate(i, __vectors[i]);
    }
    set_idt();
}

char *ex_msg[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_handler(trapframe tf) {
    kprint("Receive interrupter: ");
    char s[3];
    int_to_string(tf.int_no, s);
    kprint(s);
    kprint("   ");
    kprint(ex_msg[tf.int_no]);
    kprint("\n");
}
