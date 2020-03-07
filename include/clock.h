#pragma once

#include "types.h"

#define FREQUENCY 100

#define MIL_PER_INT (1000 / FREQUENCY)

extern u32 ticks;

void clock_init(u32 fre);

void mil_sleep(u32 m_seconds);
