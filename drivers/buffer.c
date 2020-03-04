#include "../include/buffer.h"


// 环形输入缓冲区的实现
void buffer_init(io_buffer *buffer) {
    sem_init(&buffer->x, BUFFER_SIZE);
    sem_init(&buffer->y, 0);
    buffer->head = buffer->tail = 0;
}

u32 next_pos(u32 pos) {
    return (pos + 1) % BUFFER_SIZE;
}

void buffer_getchar(io_buffer *buffer) {
    sem_down(&buffer->y);
    console_putc(buffer->buff[buffer->tail], RED);
    buffer->tail = next_pos(buffer->tail);
    sem_up(&buffer->x);
}

void buffer_putchar(io_buffer *buffer, char c) {
    sem_down(&buffer->x);

    buffer->buff[buffer->head] = c;
    buffer->head = next_pos(buffer->head);

    sem_up(&buffer->y);
}
