#include "isr.h"
#include "../driver/clock.h"

#define TICK_NUM 100

static u32 cnt = 0;

void idt_init() {
    u32 i;
    for (int i = 0; i < IRQ_END; ++i) {
        set_idt_gate(i, __vectors[i]);
    }
    set_idt();
}


char *trapname(int int_no) {
    const char *ex_msg[] = {
        "Divide error",
        "Debug",
        "Non-Maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "BOUND Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection",
        "Page Fault",
        "(unknown trap)",
        "x87 FPU Floating-Point Error",
        "Alignment Check",
        "Machine-Check",
        "SIMD Floating-Point Exception"
    };
    if (int_no < sizeof(ex_msg) / sizeof(const char *)) {
        return ex_msg[int_no];
    }
    if (int_no > IRQ_BEGIN - 1 && int_no < IRQ_END) return "PIC <---> IRQ";
    return "Unknown Trap";
}
void isr_handler(trapframe tf) {
    kprint("Interrupt: ");
    char s[3];
    int_to_string(tf.int_no, s);
    kprint(s);
    kprint("    ");
    kprint(trapname(tf.int_no));
    kprint("\n");
}

void irq_handler(trapframe tf) {
    if (tf.int_no == IRQ_TIMER) {
        if (++ticks== TICK_NUM) {
            ticks = 0;
            kprint("ticks: ");
            char s[10];
            int_to_string(++cnt, s);
            kprint(s);
            kprint("\n");
        }
    }
}

