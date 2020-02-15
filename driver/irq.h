#pragma once

#include "../libs/x86.h"

// irq 起始
#define IRQ_OFFSET 0x20

void irq_init();
void irq_enable(u32 irq);
