#pragma once

// 环形缓冲队列 -> 生产者, 消费者模式

#include "sync.h"
#include "console.h"

// 定义缓冲 区域大小 64
#define BUFFER_SIZE 0x40

typedef struct {
    semaphore x, y;
    char buff[BUFFER_SIZE];
    // 队列守卫指针
    u32 head, tail;
} io_buffer;

void buffer_init(io_buffer *buffer);

u32 next_pos(u32 pos);

void buffer_getchar(io_buffer *buffer);

void buffer_putchar(io_buffer *buffer, char c);

