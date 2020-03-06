#include "../include/pmm.h"
#include "../include/sync.h"
#include "../include/arena.h"
#include "../include/debug.h"
#include "../include/console.h"


// 申请内存时候互斥

semaphore mem_sem;

static u32 pstack[MAX_PHYSICAL_SIZE / PAGE_SIZE + 1];

u32 pstack_top;

static char kernel_bits[(KERNEL_SPACE_SIZE - 2 * PAGE_SIZE * 1024) / (PAGE_SIZE * sizeof(char))];

pool kernel_pool;

// 内核 arena 描述符
arena_desc kernel_arena[DESC_NUM];

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
    sem_init(&mem_sem, 1);
    // 内核 arena 初始化
    area_desc_init(kernel_arena);
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

// 取消 已经分配的虚拟地址 的映射
void unmap(u32 va) {
    u32 pde_id = PDE_INDEX(va);
    u32 pte_id = PTE_INDEX(va);

    u32 *pte = (u32 *)(PTE_OFFSET + (pde_id) * PAGE_SIZE);

    // 回收 物理内存, 重新装入物理内存栈
    free_page(pte[pte_id] & PAGE_MASK);
    pte[pte_id] = 0;

    // 更新 tlb
    asm volatile ("invlpg (%0)" :: "a"(va));
}

// 非同步方法, 保证 sys_malloc 同步运行
static void *malloc_page(u8 is_kernel, u32 cnt) {
    if (is_kernel) {
        int index = bitmap_apply(&kernel_pool.bmap, cnt);
        if (index == -1) return 0;
        // 释放 cnt 个 页面
        u32 base_addr = kernel_pool.addr_start + index * PAGE_SIZE;
        u32 res = base_addr;
        int i = 0;
        while (i < cnt) {
            bitmap_set(&kernel_pool.bmap, index++, 1);
            map(base_addr, kernel_pde, PG_PRESENT | PG_RW | PG_USER);
            base_addr += PAGE_SIZE;
            i++;
        }
        return (void *) res;
    } else {
        task_struct *thread = running_thread();
        int index = bitmap_apply(&thread->user_pool.bmap, cnt);
        u32 res = index * PAGE_SIZE + thread->user_pool.addr_start;
        int i = 0;
        while (i < cnt) {
            u32 va = res + (i++) * PAGE_SIZE;
            get_user_page(va);
        }
        return (void *)res;
    }
}


static void pfree_page(u8 is_kernel, u32 va, u32 cnt) {
    // 回收 虚拟地址 va 开始 的 连续页面
    int i;
    u32 index;
    if (is_kernel) {
        // 如果是内核
        index = (va - kernel_pool.addr_start) / PAGE_SIZE;
        i = 0;
        while (i < cnt) {
            unmap(va + (i++) * PAGE_SIZE);
            bitmap_set(&kernel_pool.bmap, index++, 0);
        }
    } else {
        task_struct *thread = running_thread();
        pool *p = &thread->user_pool;
        index = (va - p->addr_start) / PAGE_SIZE;
        i = 0;
        while (i < cnt) {
            unmap(va + (i++) * PAGE_SIZE);
            bitmap_set(&p->bmap, index++, 0);
        }
    }
}

void *kmalloc_page(u32 cnt, u32 *pde) {
    // 申请内存时互斥
    sem_down(&mem_sem);
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
    sem_up(&mem_sem);
    return (void *)res;
}

void *umalloc_page(u32 cnt) {
    sem_down(&mem_sem);
    task_struct *thread = running_thread();
    int index = bitmap_apply(&thread->user_pool.bmap, cnt);
    u32 res = index * PAGE_SIZE + thread->user_pool.addr_start;
    int i = 0;
    while (i < cnt) {
        u32 va = res + (i++) * PAGE_SIZE;
        get_user_page(va);
    }
    sem_up(&mem_sem);
    return (void *)res;
}

void *get_user_page(u32 va) {
    task_struct *thread = running_thread();
    // 用户页表
    map(va, (u32 *)thread->pgdir, PG_PRESENT | PG_RW | PG_USER);
    bitmap_set(&thread->user_pool.bmap, (va - thread->user_pool.addr_start) / PAGE_SIZE, 1);
    return (void *)va;
}

// 信号量不能重复 down, 由于用户栈申请, 单独提供 sync 方法
void *sync_get_user_page(u32 va) {
    sem_down(&mem_sem);
    void *res = get_user_page(va);
    sem_up(&mem_sem);
    return res;
}

u32 va2pa(u32 va) {
    // 获取真实的物理地址
    u32 pde_id = PDE_INDEX(va);
    u32 pte_id = PTE_INDEX(va);

    // 已经分配的内存, 映射0xffc00000 + 一定能获取到页表项
    u32 *pte = (u32 *)(PTE_OFFSET + (pde_id) * PAGE_SIZE);

    return (pte[pte_id] & PAGE_MASK) | (va & 0x00000fff);
}

// 系统调用 -> 进入内核态以后运行
void *pmm_malloc(u32 size) {
    // 申请内存时 互斥
    sem_down(&mem_sem);
    task_struct *thread = running_thread();
    // 区分是 用户进程 还是 内核线程
    u8 is_kernel = (u8) (thread->pgdir ? 0 : 1);
    arena *a;
    block_head *head;
    arena_desc *desc = is_kernel ? kernel_arena : thread->user_arena;
    if (size > 1024) {
        u32 page_cnt = DIV_ROUND_UP(size + sizeof(arena), PAGE_SIZE);
        a = malloc_page(is_kernel, page_cnt);
        a->desc = NULL;
        a->cnt = page_cnt;
        a->large = 1;
        // 返回 -> sem up
        sem_up(&mem_sem);
        return (void *)((u32)a + sizeof(arena));
    } else {
        // 查找应该分配 哪种块
        int i;
        for (i = 0; i < DESC_NUM; ++i) {
            if (desc[i].block_size >= size) break;
        }
        if (list_empty(&desc[i].free_list)) {
            // 如果为空 -> 则分配一页
            a = malloc_page(is_kernel, 1);
            a->desc = &desc[i];
            a->cnt = desc[i].block_nums;
            a->large = 0;
            // 关 中断
            int_status istatus = disable_int();
            int j;
            for (j = 0; j < a->cnt; ++j) {
                head = arena2block(a, (u32) j);
                list_add_last(&desc[i].free_list, &head->node);
            }
            set_int_status(istatus);
        }
        head = node2entry(block_head, node, list_pop_first(&desc[i].free_list));
        a = block2arena(head);
        a->cnt--;
        sem_up(&mem_sem);
        return (void *) head;
    }
}

void pmm_free(void *ptr) {
    // 互斥
    sem_down(&mem_sem);
    // 释放内存
    task_struct *thread = running_thread();
    // 区分是 用户进程 还是 内核线程
    u8 is_kernel = (u8) (thread->pgdir ? 0 : 1);
    block_head *head = (block_head *)ptr;
    int i;
    // 页面对齐 -> 获得该内存地址的 arena
    arena *a = block2arena(head);
    if (!a->desc && a->large) {
        // 如果是大内存块
        u32 cnt = a->cnt;
        pfree_page(is_kernel, (u32) a, cnt);
    } else {
        // 回收内存块到
        list_add_last(&a->desc->free_list, &head->node);
        if (++a->cnt == a->desc->block_nums) {
            for (i = 0; i < a->desc->block_nums; ++i) {
                head = arena2block(a, (u32) i);
                // 从 描述符 链表中删除
                list_remove(&head->node);
            }
            // 把 该页 回收了
            pfree_page(is_kernel, (u32) a, 1);
        }
    }
    sem_up(&mem_sem);
}


