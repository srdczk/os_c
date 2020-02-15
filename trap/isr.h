#pragma once

#include "idt.h"

#define ISR_BEGIN 0x00
#define ISR_END 0x20
#define IRQ_BEGIN 0x20
#define IRQ_END 0x30

extern u32 __vectors[];


// 初始化中断 服务 例程

void idt_init();
char *trapname(int int_no);
void isr_handler(trapframe tf);
void irq_handler(trapframe tf);
