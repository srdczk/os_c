#pragma once

#include "../libs/x86.h"

#define KERNEL_CS 0x08
// 目前只 支持 KERNEL 段 
#define IDT_SIZE 256
idt_gate idt[IDT_SIZE];
desc idt_desc;

void set_idt_gate(u32 n, u32 handler);
void set_idt();

