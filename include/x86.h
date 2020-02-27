#pragma once

#include "types.h"

u8 inb(u16 port);
void outb(u16 port, u8 data);
void outw(u16 port, u16 data);
void sti();
void cli();
void lidt(descriptor *desc);
u32 read_ebp();
u32 read_eip();
u32 read_eflags();
