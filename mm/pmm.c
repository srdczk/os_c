#include "../include/thread.h"
#include "../include/pmm.h"
#include "../include/debug.h"
#include "../include/console.h"

static u32 pstack[MAX_PHYSICAL_SIZE / PAGE_SIZE + 1];

u32 pstack_top;

static char kernel_bits[(KERNEL_SPACE_SIZE - 2 * PAGE_SIZE * 1024) / (PAGE_SIZE * sizeof(char))];

pool kernel_pool;

// 正式内核页目录
u32 kernel_pde[1024] __attribute__((aligned(PAGE_SIZE)));

static u32 tmp_pte[1024] __attribute__((aligned(PAGE_SIZE)));

void show_memory() {
    u32 mmap_addr = global_multiboot->mmap_addr;
	u32 mmap_length = global_multiboot->mmap_length;

	console_print("Memory map:\n");

	mmap_entry *mmap = (mmap_entry *)mmap_addr;
	for (mmap = (mmap_entry *)mmap_addr; (u32)mmap < mmap_addr + mmap_length; mmap++) {
        console_print("base_addr: 0x");
        console_print_hex((mmap->base_addr_low), GREEN);
        console_print(", length: 0x");
        console_print_hex((mmap->length_low), GREEN);
        console_print(",type: ");
        console_print_dec(mmap->type, GREEN);
        console_print("\n");

	}
}

void pstack_init() {
    pstack_top = 0;
    u32 mmap_addr = global_multiboot->mmap_addr;
	u32 mmap_length = global_multiboot->mmap_length;

	mmap_entry *mmap = (mmap_entry *)mmap_addr;
	for (mmap = (mmap_entry *)mmap_addr; (u32)mmap < mmap_addr + mmap_length; mmap++) {
        if (mmap->base_addr_low == 0x100000 && mmap->type == 0x1) {
            u32 begin = 0x400000;
            u32 end = mmap->base_addr_low + mmap->length_low;
            end = end > MAX_PHYSICAL_SIZE ? MAX_PHYSICAL_SIZE : end;
            // 全部都 对齐了 
            while (begin < end) {
                if (begin + PAGE_SIZE <= end) free_page(begin);
                begin += PAGE_SIZE;
            }
        }
	}
}

u32 alloc_page() {
    ASSERT(pstack_top);
    return pstack[--pstack_top];
}

void free_page(u32 addr) {
    pstack[pstack_top++] = addr;
}

void kernel_pool_init() {
    kernel_pool.bmap.map = kernel_bits;
    kernel_pool.bmap.map_len = (KERNEL_SPACE_SIZE - 2 * PAGE_SIZE * 1024) / (PAGE_SIZE * sizeof(char));
    kernel_pool.addr_start = 0xc0400000;
    bitmap_init(&kernel_pool.bmap);
}

void pmm_init() {
    pstack_init();
    kernel_pool_init();
    // 连连续 1024 个页表
    // 先分配 最顶端 4MB 给所有的页表
    memset((char *)kernel_pde, '\0', PAGE_SIZE);
    int i;
    for (i = 0; i < 1024; ++i) {
        tmp_pte[i] = (u32)(i << 12) | PG_PRESENT | PG_RW | PG_USER;
    }
    // 0xc0000000 -> 0xc0400000 映射到 0 - 4M
    kernel_pde[PDE_INDEX(KERNEL_PAGE_OFFSET)] = ((u32)tmp_pte - KERNEL_PAGE_OFFSET) | PG_USER | PG_RW | PG_PRESENT;
    // 页表 最后 一 位 设为 页表的物理地址, 特殊操作 0xffc00000 -> 0xffffffff 全部是页表
    kernel_pde[1023] = ((u32) kernel_pde - KERNEL_PAGE_OFFSET) | PG_PRESENT | PG_USER | PG_RW;
    // 重新分页
    asm volatile ("mov %0, %%cr3" :: "r"((u32)kernel_pde - KERNEL_PAGE_OFFSET));
}

// 新映射的
void map(u32 va, u32 *pde, u32 flags) {
    // 映射新的虚拟地址
    u32 pde_id = PDE_INDEX(va);
    u32 pte_id = PTE_INDEX(va);

    if (!pde[pde_id]) pde[pde_id] = alloc_page() | PG_USER | PG_PRESENT | PG_RW;

    u32 *pte = (u32 *)(PTE_OFFSET + (pde_id) * PAGE_SIZE);

    pte[pte_id] = alloc_page() | flags;

    // 更新 TLB
    
    asm volatile ("invlpg (%0)" :: "a"(va));
}

void *kmalloc_page(u32 cnt, u32 *pde) {
    int index = bitmap_apply(&kernel_pool.bmap, cnt);
    if (index == -1) return 0;
    // 释放 cnt 个 页面
    u32 base_addr = kernel_pool.addr_start + index * PAGE_SIZE;
    u32 res = base_addr;
    int i = 0;
    while (i < cnt) {
        bitmap_set(&kernel_pool.bmap, index++, 1);
        map(base_addr, pde, PG_PRESENT | PG_RW | PG_USER);
        base_addr += PAGE_SIZE;
        i++;
    }
    return (void *)res;
}

void *get_user_page(u32 va, task_struct *thread) {
    // 用户页表
    map(va, (u32 *)thread->pgdir, PG_PRESENT | PG_RW | PG_USER);
    return (void *)va;
}

u32 va2pa(u32 va) {
    // 获取真实的物理地址
    u32 pde_id = PDE_INDEX(va);
    u32 pte_id = PTE_INDEX(va);

    // 当前 映射0xffc00000 + 一定能获取到页表项
    u32 *pte = (u32 *)(PTE_OFFSET + (pde_id) * PAGE_SIZE);

    return (pte[pte_id] & PAGE_MASK) | (va & 0x00000fff);

}
