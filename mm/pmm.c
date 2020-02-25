#include "../include/pmm.h"
#include "../include/console.h"

// 管理最大的物理内存
u32 pmm_stack[MAX_PHYSICAL_SIZE / PAGE_SIZE + 1];
u32 physical_page_cnt;
u32 pmm_stack_size;

u32 kernel_pde[1024] __attribute__((aligned(PAGE_SIZE)));
// 基础 页表 映射 0 - 4M -> 3G -> 3G + 4M
u32 kernel_pte[1024] __attribute__((aligned(PAGE_SIZE)));



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
    pmm_stack_size = 0;
    physical_page_cnt = 0;
    // 真正 空闲的物理内存 -> 4MB 开始 是 空的
    u32 mmap_addr = global_multiboot->mmap_addr;
    u32 mmap_length = global_multiboot->mmap_length;
    mmap_entry *mmap = (mmap_entry *)mmap_addr;
    for (mmap = (mmap_entry *)mmap_addr; (u32)mmap < mmap_addr + mmap_length; mmap++) {
        // 
        // 从 1MB 开始的空 内存开始映射
        // 内核 开始映射虚拟内存
        // 可用内存段
        if (mmap->type == 1 && mmap->base_addr_low == 0x100000) {
            u32 addr = 0x400000;
            u32 end_addr = MAX_PHYSICAL_SIZE > mmap->length_low + mmap->base_addr_low ? mmap->length_low + mmap->base_addr_low : MAX_PHYSICAL_SIZE;
            while (addr < end_addr && addr + PAGE_SIZE <= end_addr) {
                physical_page_cnt++;
                free_page(addr);
                addr += PAGE_SIZE;
            }
        }
    }
}

u32 alloc_page() {
    return pmm_stack[--pmm_stack_size];
}

void free_page(u32 addr) {
    pmm_stack[pmm_stack_size++] = addr;
}

void pmm_init() {
    pstack_init();
    memset((char *)kernel_pde, '\0', sizeof(u32) * PDE_SIZE);
    memset((char *)kernel_pte, '\0', sizeof(u32) * PTE_SIZE);

    kernel_pde[PDE_INDEX(KERNEL_PAGE_OFFSET)] = ((u32)kernel_pte - KERNEL_PAGE_OFFSET) | PRESENT | WRITE;
    int i;
    for (i = 0; i < 1024; ++i) {
        kernel_pte[i] = (i << 12) | PRESENT | WRITE;
    }
    // 从 4M 开始 物理内存 是 空闲的
    // 重新映射新的 内存
    pde_enable((u32)kernel_pde - KERNEL_PAGE_OFFSET);
}

void pde_enable(u32 pde_addr) {
    // 真实的物理地址
    asm volatile ("mov %0, %%cr3" :: "r"(pde_addr));
}

// 映射 物理页面
void map(u32 *pded, u32 va, u32 pa, u32 flag) {
    u32 pde_id = PDE_INDEX(va);
    u32 pte_id = PTE_INDEX(va);
    
    u32 pte_addr = pded[pde_id] & PAGE_MASK;
    if (!pte_addr) {
        // 没有 分配
        pte_addr = alloc_page();
        pded[pde_id] = pte_addr | PRESENT | WRITE;
        pte_addr += KERNEL_PAGE_OFFSET;
        memset((char *)pte_addr, '\0', PAGE_SIZE);
    } else {
        pte_addr += KERNEL_PAGE_OFFSET;
    }
    u32 *pted = (u32 *)pte_addr;
    pted[pte_id] = (pa & PAGE_MASK) | flag;

    asm volatile ("invlpg (%0)" :: "a"(va));
}
// 解除映射
void unmap(u32 *pded, u32 va) {
    u32 pde_id = PDE_INDEX(va);
    u32 pte_id = PTE_INDEX(va);
    
    u32 pte = pded[pde_id] & PAGE_MASK;
    if (!pte) return;

    u32 *pted = (u32 *)(pte + KERNEL_PAGE_OFFSET);

    memset((char *)(pted + pte_id), '\0', sizeof(u32));

    asm volatile ("invlpg (%0)" :: "a"(va));
}

