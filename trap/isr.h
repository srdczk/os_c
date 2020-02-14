#pragma once

#include "idt.h"

extern u32 __vectors[];

// 初始化中断 服务 例程

void isr_install();
void isr_handler(trapframe tf);
