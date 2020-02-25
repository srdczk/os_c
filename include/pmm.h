#pragma once


#include "../include/multiboot.h"

#define PRESENT 0x1

#define WRITE 0x2
// 管理的最大物理内存
#define MAX_PHYSICAL_SIZE 0x20000000
// 页面大小
#define PAGE_SIZE 0x1000
// 页面掩码, 按照 4k 对齐
#define PAGE_MASK 0xfffff000

#define KERNEL_PAGE_OFFSET 0xc0000000

#define PDE_INDEX(x) (((x) >> 22) & 0x3ff)

#define PTE_INDEX(x) (((x) >> 12) & 0x3ff)

#define PTE_SIZE 1024

#define PDE_SIZE 1024

extern u8 kernel_start[];
extern u8 kernel_end[];

extern u32 kernel_pde[];

extern u32 physical_page_cnt;

extern u32 pmm_stack_size;

void show_memory();

void pstack_init();

u32 alloc_page();

void free_page(u32 addr);

void pmm_init();

void pde_enable(u32 pde_addr);

void map(u32 *pded, u32 va, u32 pa, u32 flag);

void unmap(u32 *pded, u32 va);



