#include "pmm.h"

extern u32 placement_addr;

page_directory_entry *kernel_entry = 0;
page_directory_entry *cur_entry = 0;

// 用bit图来表示 物理内存的分配情况 
// 假设 物理内存 16M
//
#define PHYSICAL_MEMORY 0x1000000
#define PAGE_SIZE 0x1000

char *frames;
u32 frames_size;

static void set_frame(u32 frame_addr) {
    u32 index = frame_addr / 0x1000;
    // 页面 ->
    frames[index] |= 0x01;
}

static void clear_frame(u32 frame_addr) {
    u32 index = frame_addr / 0x1000;
    frames[index] &= 0x00;
} 

static char test_frame(u32 frame_addr) {
    u32 index = frame_addr / 0x1000;
    return frames[index];
}
static u32 first_frame() {
    int i;
    for (i = 0; i < frames_size; ++i) {
        if (!frames[i]) return i;
    }
    return i;
}

void alloc_frame(page_entry *page, int is_kernel, int is_writeable) {
    // 如果 page 的 帧不是 0
    if (page->frame) return;
    u32 index = first_frame();
    if (index == frames_size) return;
    set_frame(index * 0x1000);
    page->present = 1;
    page->rw = is_writeable ? 1 : 0;
    page->user = is_kernel ? 0 : 1;
    page->frame = index & 0xfffff;
}

void free_frame(page_entry *page) {
    u32 frame;
    // 如果 帧是 0
    if (!(frame = page->frame)) return;
    clear_frame(frame * 0x1000);
    page->frame = 0x00;
}

// 使能分页 -> 先将 底下的内核代码 映射好, 使得 gdt idt vram 等正常使用

void page_init() {
    // nframes 从 1MB  0x100000 开始申请, -> 内核代码从0x9000 开始加载
    frames_size = PHYSICAL_MEMORY / PAGE_SIZE;
    frames = (char *)kmalloc(frames_size, 0, 0);
    memset(frames, 0, frames_size);
    kernel_entry = (page_directory_entry *)kmalloc(sizeof(page_directory_entry), 1, 0);
    memset((char *)kernel_entry, 0, sizeof(page_directory_entry));
    cur_entry = kernel_entry;
    // 映射第一个 页表 0 - 4M 全部映射上去
    u32 first_block = 4 * 1024 * 1024;
    u32 tmp;
    page_table_entry *first_table = (page_table_entry *)kmalloc(sizeof(page_table_entry), 1, &tmp);
    memset((char *)first_table, 0, sizeof(page_table_entry));
    kernel_entry->tables[0] = first_table;
    kernel_entry->tablePhysicals[0] = tmp | 0x7;
    int i = 0, cnt = 0;
    while (i < first_block) {
        // 多加一个页面
        set_frame(i);
        page_entry *page = &kernel_entry->tables[0]->pages[cnt];
        page->present = 1;
        page->rw = 0;
        page->user = 1;
        page->frame = ((i >> 12) & 0xfffff);
        i += 0x1000;
        ++cnt;
    }
    enable_directory(kernel_entry);
}

void enable_directory(page_directory_entry *entry) {
    cur_entry = entry;
    asm volatile("mov %0, %%cr3" :: "r"(&entry->tablePhysicals));
    u32 cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
    // 分页完成
}

page_entry *get_page(u32 addr, int make, page_directory_entry *entry) {
    // 真实物理地址 分配 页面
    addr /= PAGE_SIZE;
    u32 index = addr / 1024;
    if (entry->tables[index]) {
        return &entry->tables[index]->pages[addr % 1024];
    } else if (make) {
        u32 tmp;
        entry->tables[index] = (page_table_entry *)kmalloc(sizeof(page_table_entry), 1, &tmp);
        memset(entry->tables[index], 0, 0x1000);
        entry->tablePhysicals[index] = tmp | 0x7;
        return &entry->tables[index]->pages[addr % 1024];
    } else return 0;
}
