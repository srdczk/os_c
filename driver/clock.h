#pragma once

#include "irq.h"
#include "../libs/string.h"

extern u32 ticks;

#define IO_TIMER1       0x40


#define TIMER_FREQ      1193182
#define TIMER_DIV(x)    ((TIMER_FREQ + (x) / 2) / (x))

#define TIMER_MODE      (IO_TIMER1 + 3) 
#define TIMER_SEL0      0x00        
#define TIMER_RATEGEN   0x04            
#define TIMER_16BIT     0x30
#define IRQ_TIMER       0x20

void clock_init();
