#pragma once

#include "../libs/x86.h"

#define KERNEL_CS 0x08
// 目前只 支持 KERNEL 段 
// 中断门
typedef struct {
    u16 low_offset;
    u16 selector;
    u8 always0;
    u8 flags;
    u16 high_offset;
}__attribute__((packed))idt_gate;
// 禁用编译器 字节对齐功能
// base + limit
#define IDT_SIZE 256
idt_gate idt[IDT_SIZE];
desc idt_desc;

void set_idt_gate(int n, u32 handler);
void set_idt();

