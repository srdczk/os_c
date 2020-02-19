#pragma once

#include "x86.h"

extern u32 placement_addr;

u32 kmalloc(u32 size, int align, u32 *phy);
