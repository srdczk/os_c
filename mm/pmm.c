#include "../include/pmm.h"
#include "../include/debug.h"
#include "../include/console.h"

static u32 pstack[MAX_PHYSICAL_SIZE / PAGE_SIZE + 1];

u32 pstack_top;

static char kernel_bits[(KERNEL_SPACE_SIZE - 2 * PAGE_SIZE * 1024) / (PAGE_SIZE * sizeof(char))];

pool kernel_pool;

static u32 tmp_kernel_pde[1024] __attribute__((aligned(PAGE_SIZE)));

static u32 tmp_pte_low[1024] __attribute__((aligned(PAGE_SIZE)));

static u32 tmp_pte_high[1024] __attribute__((aligned(PAGE_SIZE)));

static u32 kernel_pde[1024] __attribute__((aligned(PAGE_SIZE)));

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
    int i;
    for (i = 0; i < 1024; ++i) {
        tmp_pte_low[i] = (i << 12) | PG_PRESENT | PG_RW;
    }
    for (i = 0; i < 1024; ++i) {
        tmp_pte_high[i] = alloc_page() | PG_PRESENT | PG_RW;
    }
    tmp_kernel_pde[PDE_INDEX(KERNEL_PAGE_OFFSET)] = ((u32)tmp_pte_low - KERNEL_PAGE_OFFSET) | PG_PRESENT | PG_RW;
    tmp_kernel_pde[1023] = ((u32)tmp_pte_high - KERNEL_PAGE_OFFSET) | PG_PRESENT | PG_RW;
    // 重新分页
    asm volatile ("mov %0, %%cr3" :: "r"((u32)tmp_kernel_pde - KERNEL_PAGE_OFFSET));
    // 已经分配 的 0xffc00000 - 0xffffffff 作为 所有pte, 映射到真正的pde中
    for (i = 0; i < 1024; ++i) {
    //    u32 addr = PTE_OFFSET + i * PAGE_SIZE;
    //    console_print_hex(addr, GREEN);console_print("\n");
        char *start = (char *)(PTE_OFFSET + i * PAGE_SIZE);
        memset(start, '\0', PAGE_SIZE);
        u32 *begin = (u32 *)(PTE_OFFSET + i * PAGE_SIZE);
        // 真正的物理地址
        kernel_pde[i] = tmp_pte_high[i];
        int j;
        if (i == PDE_INDEX(KERNEL_PAGE_OFFSET)) {
            memcpy((char *)tmp_pte_low, start, PAGE_SIZE);
        } else if (i == 1023) {
            memcpy((char *)tmp_pte_high, start, PAGE_SIZE);;
        }
    }
    asm volatile ("mov %0, %%cr3" :: "r"((u32)kernel_pde - KERNEL_PAGE_OFFSET));
}

// 新映射的
void map(u32 va, u32 flags) {
    u32 addr = alloc_page();
    u32 pde_id = PDE_INDEX(va);
    u32 pte_id = PTE_INDEX(va);

    u32 *pte = (u32 *)(PTE_OFFSET + (pde_id) * PAGE_SIZE);

    pte[pte_id] = addr | flags;

    // 更新 TLB
    
    asm volatile ("invlpg (%0)" :: "a"(va));
}

void *malloc_page(pool_flag pf, u32 cnt) {
    if (pf == PF_KERNEL) {

        int index = bitmap_apply(&kernel_pool.bmap, cnt);
        if (index == -1) return 0;
        // 释放 cnt 个 页面
        u32 base_addr = kernel_pool.addr_start + index * PAGE_SIZE;
        u32 res = base_addr;
        int i = 0;
        while (i < cnt) {
            bitmap_set(&kernel_pool.bmap, index++, 1);
            map(base_addr, PG_PRESENT | PG_RW);
            base_addr += PAGE_SIZE;
            i++;
        }
        return (void *)res;
    } else return 0;
}
