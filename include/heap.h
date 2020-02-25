#pragma once

#include "types.h"

#define HEAP_START 0xc0100000

typedef struct block {
    struct block *pre;
    struct block *next;
    unsigned allocated: 1;// 当前内存块是否被分配 -> 一个 bit
    unsigned length:    31;// 当前内存块 的长度
} block;

void heap_init();

void *kmalloc(u32 len);

void kfree(void *p);
