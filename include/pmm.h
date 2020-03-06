#pragma once

#include "arena.h"
#include "bitmap.h"
#include "multiboot.h"

#define PG_PRESENT 0x1

#define PG_RW 0x2

#define PG_USER 0x4

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

#define KERNEL_SPACE_SIZE 0x40000000

#define PTE_OFFSET 0xffc00000
// 向上取整
#define DIV_ROUND_UP(X, Y) ((X + Y - 1) / (Y))

extern u8 kernel_start[];
extern u8 kernel_end[];
extern u32 kernel_pde[];
//extern semaphore mem_sem;
typedef struct {
    bitmap bmap;
    u32 addr_start;
} pool;

typedef enum {
    PF_KERNEL,
    PF_USER
} pool_flag;

extern u32 pstack_top;

extern pool kernel_pool;

void show_memory();

void pstack_init();

u32 alloc_page();

void free_page(u32 addr);

void kernel_pool_init();

void pmm_init();

void map(u32 va, u32 *pde, u32 flags);

void unmap(u32 va);

void *kmalloc_page(u32 cnt, u32 *pde);

void *umalloc_page(u32 cnt);

void *get_user_page(u32 va);

void *sync_get_user_page(u32 va);

u32 va2pa(u32 va);

void *pmm_malloc(u32 size);

void pmm_free(void *ptr);

