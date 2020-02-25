#pragma once

#include "x86.h"
#include "string.h"

#define CURSOR_DATA 0x3d5
#define CURSOR_CTRL 0x3d4

#define MAX_ROW 25
#define MAX_COL 80

// 定义使用颜色输出
#define WHITE 0x0f
#define RED 0x0c
#define GREEN 0x0a
#define MAGENTA 0x0d

void console_clear();

void console_putc(char c, u8 color);

void console_print(char *s);

void console_print_color(char *s, u8 color);

void console_print_hex(u32 n, u8 color);

void console_print_dec(u32 n, u8 color);


