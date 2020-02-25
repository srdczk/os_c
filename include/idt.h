#pragma once

#include "types.h"

// 中断门
typedef struct {
    u16 low_offset;
    u16 selector;
    u8 always0;
    u8 flags;
    u16 high_offset;
} __attribute__((packed)) idt_gate;

typedef struct {
    u32 ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, useresp, ss;
} int_frame;

void set_idt_gate(u32 num, u32 handler);
void set_idt();


