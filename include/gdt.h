#pragma once

#include "types.h"


#define GDT_SIZE 6

#define SEG_NULL    0
#define SEG_KTEXT   1
#define SEG_KDATA   2
#define SEG_UTEXT   3
#define SEG_UDATA   4
#define SEG_TSS     5
// 内核代码段
#define GD_KTEXT    ((SEG_KTEXT) << 3)
// 内核数据段
#define GD_KDATA    ((SEG_KDATA) << 3)
// 用户代码段
#define GD_UTEXT    ((SEG_UTEXT) << 3)
// 用户数据段
#define GD_UDATA    ((SEG_UDATA) << 3)
// 任务段
#define GD_TSS      ((SEG_TSS) << 3)

#define DPL_KERNEL  (0)
#define DPL_USER    (3)

#define KERNEL_CS   ((GD_KTEXT) | DPL_KERNEL)
#define KERNEL_DS   ((GD_KDATA) | DPL_KERNEL)
#define USER_CS     ((GD_UTEXT) | DPL_USER)
#define USER_DS     ((GD_UDATA) | DPL_USER)


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

typedef struct {
    u32 base_link;
    u32 esp0;
    u16 ss0;
    u16 padding1;
    u32 esp1;
    u16 ss1;
    u16 padding2;
    u32 esp2;
    u16 ss2;
    u16 padding3;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u16 es;
    u16 padding4;
    u16 cs;
    u16 padding5;
    u16 ss;
    u16 padding6;
    u16 ds;
    u16 padding7;
    u16 fs;
    u16 padding8;
    u16 gs;
    u16 padding9;
    u16 ldt;
    u16 padding10;
    u16 trace;
    u16 io_base;
} __attribute__((packed)) tss_entry;

void gdt_init();

void update_esp0(u32 esp0);

u32 get_esp0();



