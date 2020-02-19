#include "mem.h"

// 起始分配 1MB -> 不影响gdt, vram 等
u32 placement_addr = 0x100000;

u32 kmalloc(u32 size, int align, u32 *phy) {
    if (align && (placement_addr & 0xfffff000)) {
        // 页面对齐, 每一页大小 0x1000
        placement_addr &= 0xfffff000;
        placement_addr += 0x1000;
    }
    if (phy) *phy = placement_addr;
    u32 res = placement_addr;
    placement_addr += size;
    return res;
}
