#pragma once

#include "types.h"

u8 inb(u16 port);

void outb(u16 port, u8 data);

void outsw(u16 port, const void* addr, u32 word_cnt);

void insw(u16 port, void* addr, u32 word_cnt);

void outw(u16 port, u16 data);
void sti();
void cli();
void lidt(descriptor *desc);
u32 read_esp();
u32 read_ebp();
u32 read_eip();
u32 read_eflags();
