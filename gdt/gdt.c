#include "../include/gdt.h"


#define GDT_SIZE 5

gdt_entry gdt[GDT_SIZE];

descriptor gdt_desc;

// gdt flush 汇编实现  -> lgdt 并设置相关寄存器
extern void gdt_flush(descriptor *desc);

static void set_gdt_entry(u32 num, u32 base, u32 limit, u8 access, u8 gran) {
    gdt[num].base_low = base & 0xffff;
    gdt[num].base_middle = (base >> 16) & 0xff;
    gdt[num].base_high = (base >> 24) & 0xff;
    gdt[num].limit_low = limit & 0xffff;
    gdt[num].granularity = (limit >> 16) & 0x0f;
    gdt[num].granularity |= gran & 0xf0;
    gdt[num].access = access;
}

void  gdt_init() {
    // 设置 null 段, 内核代码段, 内核数据段, 用户代码段, 用户数据段
    gdt_desc.limit = sizeof(gdt_entry) * GDT_SIZE - 1;
    gdt_desc.base = (u32)&gdt;

    set_gdt_entry(0, 0, 0, 0, 0);
    set_gdt_entry(1, 0, 0xffffffff, 0x9a, 0xcf);
    set_gdt_entry(2, 0, 0xffffffff, 0x92, 0xcf);
    set_gdt_entry(3, 0, 0xffffffff, 0xfa, 0xcf);
    set_gdt_entry(4, 0, 0xffffffff, 0xf2, 0xcf);

    gdt_flush(&gdt_desc);

}

