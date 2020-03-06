#pragma once

#include "types.h"
#include "list.h"

// 7种描述符 16 -> 32 -> 64 -> 128 -> 256 -> 512 -> 1024
#define DESC_NUM 7

// 每一个块头部在未分配的时候维持一个 链表节点
typedef struct {
    list_node node;
} block_head;

typedef struct {
    // 块的大小
    u32 block_size;
    // 可以容纳的数量
    u32 block_nums;
    list free_list;
} arena_desc;

// 申请 超过 1024 大小的内存 -> 直接分配页
typedef struct {
    // 描述指针
    arena_desc *desc;
    u32 cnt;
    u8 large;
} arena;

void area_desc_init(arena_desc *desc);

block_head *arena2block(arena *a, u32 i);

arena *block2arena(block_head *head);




