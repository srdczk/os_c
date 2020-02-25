#pragma once

#include "types.h"
// 禁用字节对齐
// gdt 表项
typedef struct {
    u16 limit_low;
    u16 base_low;
    u8 base_middle;
    u8 access;
    u8 granularity;
    u8 base_high;
} __attribute__((packed)) gdt_entry;

void gdt_init();



