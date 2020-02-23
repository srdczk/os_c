#pragma once


#include "../include/multiboot.h"

#define PDE_SIZE 1024

#define PTE_SIZE 1024

#define KERNEL_PTE 128

#define PDE_INDEX(x) (((x) >> 22) & 0x3ff)

#define PTE_INDEX(x) (((x) >> 12) & 0x3ff)

#define PRESENT 0x1

#define WRITE 0x2
// 管理的最大物理内存
#define MAX_PHYSICAL_SIZE 0x20000000
// 页面大小
#define PAGE_SIZE 0x1000
// 页面掩码, 按照 4k 对齐
#define PAGE_MASK 0xfffff000

extern u8 kernel_start[];
extern u8 kernel_end[];

extern u32 physical_page_cnt;

extern u32 pmm_stack_size;

extern u32 pde[];

void show_memory();

void pmm_init();

u32 alloc_page();

void free_page(u32 addr);

void map(u32 *pd, u32 va, u32 flags);

void unmap(u32 *pd, u32 va);
