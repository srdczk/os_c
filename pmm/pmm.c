#include "../include/pmm.h"
#include "../include/console.h"
#include "../include/string.h"

// 管理最大的物理内存

u32 pmm_stack[MAX_PHYSICAL_SIZE / PAGE_SIZE + 1];
u32 physical_page_cnt;
u32 pmm_stack_size;

// ld 链接脚本中定义的变量, 用来计算 kern 的长度

// 申请 内核页表 -> 最大 PTE 管理内存 512 MB -> 128 PTE 
u32 pde[PDE_SIZE]__attribute__((aligned(PAGE_SIZE)));
static u32 ptes[PDE_SIZE][PTE_SIZE]__attribute__((aligned(PAGE_SIZE)));


void show_memory() {
    u32 mmap_addr = glb_mboot_ptr->mmap_addr;
	u32 mmap_length = glb_mboot_ptr->mmap_length;

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


void pmm_init() {
    pmm_stack_size = 0;
    physical_page_cnt = 0;
    u32 mmap_addr = glb_mboot_ptr->mmap_addr;
    u32 mmap_length = glb_mboot_ptr->mmap_length;
    mmap_entry *mmap = (mmap_entry *)mmap_addr;
    for (mmap = (mmap_entry *)mmap_addr; (u32)mmap < mmap_addr + mmap_length; mmap++) {
        // 
        // 从 1MB 开始的空 内存开始映射
        // 内核 开始映射虚拟内存
        // 可用内存段
        if (mmap->type == 1 && mmap->base_addr_low == 0x100000) {
            u32 addr = mmap->base_addr_low + (u32)kernel_end - (u32)kernel_start;
            u32 end_addr = MAX_PHYSICAL_SIZE > mmap->length_low + mmap->base_addr_low ? mmap->length_low + mmap->base_addr_low : MAX_PHYSICAL_SIZE;
            while (addr < end_addr && addr + PAGE_SIZE <= end_addr) {
                physical_page_cnt++;
                free_page(addr);
                addr += PAGE_SIZE;
            }
        }
    }
    // 开启分页
    memset(pde, '\0', sizeof(u32));
    int i;
    for (i = 0; i < PDE_SIZE; ++i) {
        memset(ptes[i], '\0', sizeof(u32) * PTE_SIZE);
    }
    u32 end = (u32)kernel_end;//由于 全部都 4K 对齐了
    u32 begin = 0;
    while (begin < end) {
        pde[PDE_INDEX(begin)] = (u32)ptes[PDE_INDEX(begin)] | PRESENT | WRITE;
        ptes[PDE_INDEX(begin)][PTE_INDEX(begin)] = begin | PRESENT | WRITE;
        begin += PAGE_SIZE;
    }
    // 内核用到的部分已经完全映射了 
    // 开启分页
    asm volatile("mov %0, %%cr3" :: "r"(pde));
    u32 cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
    // 已经开启分页    
}

u32 alloc_page() {
    return pmm_stack[--pmm_stack_size];
}

void free_page(u32 addr) {
    pmm_stack[pmm_stack_size++] = addr;
}

void map(u32 *pd, u32 va, u32 flags) {
    // 新分配页面
    u32 pde_id = PDE_INDEX(va);
    u32 pte_id = PTE_INDEX(va);
    // 分配一个真实物理页面
    pd[pde_id] = (u32)ptes[pde_id] | PRESENT | WRITE;
    if (!ptes[pde_id][pte_id]) {
        u32 addr = alloc_page();
        ptes[pde_id][pte_id] = (addr & PAGE_MASK) | flags;
    }
    flush_tlb(va);
}

void unmap(u32 *pd, u32 va) {
    // free page;
    u32 pde_id = PDE_INDEX(va);
    u32 pte_id = PTE_INDEX(va);
    if (!ptes[pde_id][pte_id]) return;
    u32 addr = ptes[pde_id][pte_id] & PAGE_MASK;
    memset((char *)&ptes[pde_id][pte_id], '\0', sizeof(u32));
    free_page(addr);
    flush_tlb(va);
}

