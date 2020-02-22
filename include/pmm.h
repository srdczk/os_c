#pragma once


#include "../include/multiboot.h"

// 管理的最大物理内存
#define MAX_PHYSICAL_SIZE 0x20000000
// 页面大小
#define PAGE_SIZE 0x1000
// 页面掩码, 按照 4k 对齐
#define PAGE_MASK 0xfffff000

extern u8 kern_start[];
extern u8 kern_end[];

extern u32 physical_page_cnt;

extern u32 pmm_stack_size;

void show_memory();

void pmm_init();

u32 alloc_page();

void free_page(u32 addr);


