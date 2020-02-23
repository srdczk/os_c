#include "../include/heap.h"
#include "../include/pmm.h"

// 每一个 内存块的大小


static block *begin;

static void alloc_block(u32 start, u32 len);

static void free_block(block *b);

static void split_block(block *b, u32 len);

static void union_block(block *b);

static u32 heap_max = HEAP_START;

void heap_init() {
    begin = 0;
}

void *kmalloc(u32 len) {
    // 申请的内存中都要 保存 block 信息结构体
    len += sizeof(block);
    block *cur = begin;
    block *pre = 0;
    // 遍历链表 -> 找到能够分配的节点 
    while (cur) {
        if (!cur->allocated && cur->length >= len) {
            split_block(cur, len);
            cur->allocated = 1;
            return (void *)((u32)cur + sizeof(block));
        }
        pre = cur;
        cur = cur->next;
    }
    // 如果遍历都找不到 
    u32 block_start;
    if (pre) {
        block_start = (u32)pre + pre->length;
    } else {
        // 说明第一次运行 -> 啥都没有
        block_start = HEAP_START;
        begin = (block *)block_start;
    }

    alloc_block(block_start, len);
    cur = (block *)block_start;
    cur->pre = pre;
    cur->next = 0;
    cur->allocated = 1;
    cur->length = len;
    if (pre) {
        pre->next = cur;
    }
    return (void *)(block_start + sizeof(block));
}

void kfree(void *p) {
    block *b = (block *)((u32)p - sizeof(block));
    b->allocated = 0;
    union_block(b);
}

void alloc_block(u32 start, u32 len) {
    while (start + len > heap_max) {
        map(pde, heap_max, PRESENT | WRITE);
        heap_max += PAGE_SIZE;
    }
}

void free_block(block *b) {
    // 释放最后一块 内存块
    if (!b->pre) begin = 0;
    else b->pre->next = 0;
    // 空闲的内存超过 一页
    while ((heap_max - PAGE_SIZE) >= (u32)b) {
        heap_max -= PAGE_SIZE;
        // 释放 visual addr
        unmap(pde, heap_max);
    }
}

void split_block(block *b, u32 len) {
    if (b->length - len > sizeof(block)) {
        block *new_block = (block *)((u32)b + len);
        new_block->pre = b;
        new_block->next = b->next;
        new_block->allocated = 0;
        new_block->length = b->length - len;

        b->next = new_block;
        b->length = len;
    }
}

void union_block(block *b) {
    if (b->next && !b->next->allocated) {
        b->length += b->next->length;
        if (b->next->next) b->next->next->pre = b;
        b->next = b->next->next;
    }

    if (b->pre && !b->pre->allocated) {
        b->pre->length += b->length;
        b->pre->next = b->next;
        if (b->next) b->next->pre = b->pre;
        b = b->pre;
    }
    if (!b->next) free_block(b);
}


