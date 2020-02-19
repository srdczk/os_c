#include "isr.h"
#include "../driver/clock.h"
#include "../driver/keyboard.h"

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
    if (tf.int_no == 14) {
        // 缺页中断
        u32 fault_addr;
        asm volatile("mov %%cr2, %0": "=r"(fault_addr));
        kprint("Page Fault: ");
        char s[10];
        int_to_string(fault_addr, s);
        kprint(s);
        kprint("\n");
    } else {
        kprint("Interrupt: ");
        char s[3];
        int_to_string(tf.int_no, s);
        kprint(s);
        kprint("    ");
        kprint(trapname(tf.int_no));
        kprint("\n");
    }
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
    } else if (tf.int_no == IRQ_BEGIN + 1) {
        // 键盘中断
        // 读取 扫描码
        u8 code = inb(0x60);
        outb(0x20, 0x20);
        // 发送 EOI -> 8529
        print_letter(code);
    }
}

