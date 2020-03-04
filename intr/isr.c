#include "../include/isr.h"
#include "../include/thread.h"
#include "../include/console.h"
#include "../include/clock.h"
#include "../include/debug.h"
#include "../include/keyboard.h"
#include "../include/string.h"
#include "../include/syscall.h"

#define IRQ_BEGIN 0x20
#define IRQ_END 0x30
#define TIMER_NUM 100

#define PAGE_FAULT 0x0e
// 如果开启了中断 eflags 第 9 位设 为 1
#define IF_EFLAGS 0x00000200
#define KEY_PORT 0x60

static u32 cnt = 0;

extern u32 isrs[];

int_status get_int_status() {
    return (read_eflags() & IF_EFLAGS) ? INT_ON : INT_OFF;
}

// 返回 中断前的状态
int_status set_int_status(int_status status) {
    return status ? enable_int() : disable_int();
}

int_status enable_int() {
    if (get_int_status()) return INT_ON;
    else {
        sti();
        return INT_OFF;
    }
}

int_status disable_int() {
    if (get_int_status()) {
        cli();
        return INT_ON;
    } else return INT_OFF;
}

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
        set_idt_gate((u32) i, isrs[i], 0x08, 0x8e);
    }
    // 设置 系统调用, DPL = 3
    set_idt_gate(T_SYSCALL, isrs[48], 0x08, 0xef);
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
        if (tf->int_no == PAGE_FAULT) {
            u32 cr2;
            asm volatile("mov %%cr2, %0" : "=r"(cr2));
            char s[9];
            to_hex_string(cr2, s);
            s[8] = '\0';
            char msg[21];
            msg[0] = '\0';
            strcat("Page fault at:0x", msg);
            strcat(s, msg);
            PANIC(msg);
        }
    } else if (tf->int_no < 0x80) {
        if (tf->int_no >= 40) outb(0xa0, 0x20);
        outb(0x20, 0x20);
        if (tf->int_no == IRQ_BEGIN) {
            schedule();
        } else if (tf->int_no == IRQ_BEGIN + 1) {
            keyboard_handler(inb(KEY_PORT));
        }
    } else syscall(tf);
}

