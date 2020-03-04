#pragma once

#include "types.h"

// 使用 0x80 号中断作系统调用
#define T_SYSCALL 0x80

// 中断门
typedef struct {
    u16 low_offset;
    u16 selector;
    u8 always0;
    u8 flags;
    u16 high_offset;
} __attribute__((packed)) idt_gate;

typedef struct {
    // 中断处理中 pusha
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    // 16 位 寄存器 带占位符
    u16 gs, padding0;
    u16 fs, padding1;
    u16 es, padding2;
    u16 ds, padding3;
    // 中断号
    u32 int_no;
    // 特定中断号 cpu 自动push, 如果没有中断处理中 push 了一个占位的
    u32 error_code;
    // int 指令 push, 为了还原中断现场
    u32 eip;
    u16 cs, padding4;
    u32 eflags;
    // 用户态中断, push
    u32 useresp;
    u16 ss, padding5;
} __attribute__((packed)) int_frame;

void set_idt_gate(u32 num, u32 handler, u16 sel, u8 flags);
void set_idt();


