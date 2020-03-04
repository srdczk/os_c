#pragma once

#include "types.h"
#include "buffer.h"
// 键盘按下 -> 发送 通码, 键盘释放 发送断码 根据标准处理标准键盘 和 shift + xxx

extern io_buffer kb_buffer;

void kbuffer_init();

void keyboard_handler(u8 code);
