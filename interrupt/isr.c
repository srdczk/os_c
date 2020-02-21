#include "../include/isr.h"
#include "../include/console.h"
#include "../include/clock.h"
#define IRQ_BEGIN 0x20
#define IRQ_END 0x30
#define TIMER_NUM 100

static u32 cnt = 0;

extern u32 isrs[];

void idt_init() {
	// 初始化主片、从片
	// 0001 0001
	outb(0x20, 0x11);
	outb(0xa0, 0x11);

	outb(0x21, 0x20);

	outb(0xa1, 0x28);

	outb(0x21, 0x04);

	outb(0xa1, 0x02);

	outb(0x21, 0x01);
	outb(0xa1, 0x01);

	outb(0x21, 0x0);
	outb(0xa1, 0x0);
    int i;
    for (i = 0; i < 48; ++i) {
        set_idt_gate(i, isrs[i]);
    }
    set_idt();
}

const char *int_name(int int_no) {
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

void int_dispatch(int_frame *tf) {
    if (tf->int_no < IRQ_BEGIN) {
        console_print(int_name(tf->int_no));
        console_print("\n");
    } else {
        if (tf->int_no >= 40) outb(0xa0, 0x20);
        outb(0x20, 0x20);
        if (tf->int_no == IRQ_BEGIN) {
            if (++ticks == TIMER_NUM) {
                console_print("tick: ");
                console_print_dec(++cnt, GREEN);
                console_print("\n");
                ticks = 0;
            }
        } else if (tf->int_no == IRQ_BEGIN + 1) {
            u8 code = inb(0x60);
            console_print("Keyboard: ");
            console_print_dec(code, RED);
            console_print("\n");
        }
    }
}
