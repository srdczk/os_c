#include "../include/gdt.h"
#include "../include/string.h"


static gdt_entry gdt[GDT_SIZE];

static descriptor gdt_desc;

static tss_entry tss;
// gdt flush 汇编实现  -> lgdt 并设置相关寄存器
extern void gdt_flush(descriptor *desc);

static void set_gdt_entry(u32 num, u32 base, u32 limit, u8 access, u8 gran) {
    gdt[num].base_low = (u16) (base & 0xffff);
    gdt[num].base_middle = (u8) ((base >> 16) & 0xff);
    gdt[num].base_high = (u8) ((base >> 24) & 0xff);
    gdt[num].limit_low = (u16) (limit & 0xffff);
    gdt[num].granularity = (u8) ((limit >> 16) & 0x0f);
    gdt[num].granularity |= gran & 0xf0;
    gdt[num].access = access;
}

static void set_tss_entry(u32 num, u16 ss0, u16 esp0) {
    u32 base = (u32) &tss;
    u32 limit = base + sizeof(tss_entry);

    set_gdt_entry(num, base, limit, 0xe9, 0x00);
    memset((char *)&tss, '\0', sizeof(tss_entry));
    // 用于 从 3 -> 0 使用 esp0
    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.cs = GD_KTEXT | DPL_USER;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = GD_KDATA | DPL_USER;
}

void  gdt_init() {
    // 设置 null 段, 内核代码段, 内核数据段, 用户代码段, 用户数据段
    gdt_desc.limit = sizeof(gdt_entry) * GDT_SIZE - 1;
    gdt_desc.base = (u32)&gdt;

    set_gdt_entry(SEG_NULL, 0, 0, 0, 0);
    set_gdt_entry(SEG_KTEXT, 0, 0xffffffff, 0x9a, 0xcf);
    set_gdt_entry(SEG_KDATA, 0, 0xffffffff, 0x92, 0xcf);
    set_gdt_entry(SEG_UTEXT, 0, 0xffffffff, 0xfa, 0xcf);
    set_gdt_entry(SEG_UDATA, 0, 0xffffffff, 0xf2, 0xcf);

    set_tss_entry(SEG_TSS, GD_KDATA, 0);

    gdt_flush(&gdt_desc);

    asm volatile ("mov $0x2b, %ax;"
                  "ltr %ax;");
}

void update_esp0(u32 esp0) {
    tss.esp0 = esp0;
}

