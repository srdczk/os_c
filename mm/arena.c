#include "../include/arena.h"


void area_desc_init(arena_desc *desc) {
    // 初始的块大小
    u32 block_size = 16;
    int i;
    for (i = 0; i < DESC_NUM; ++i) {
        desc[i].block_size = block_size;
        // 每一页 的头部作为 arena
        desc[i].block_nums = (0x1000 - sizeof(arena)) / block_size;
        list_init(&desc[i].free_list);
        block_size *= 2;
    }
}

// arena 中第 i 个块的起始地址
block_head *arena2block(arena *a, u32 i) {
    return (block_head *) ((u32)a + sizeof(arena) + i * a->desc->block_size);
}

// arena 的地址, 页对齐
arena *block2arena(block_head *head) {
    return (arena *)((u32)head & 0xfffff000);
}