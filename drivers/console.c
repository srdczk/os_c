#include "../include/console.h"

static u8 *vram = (char *)0xb8000;

static u8 cursor_x = 0;
static u8 cursor_y = 0;


// 内部实现的
static void set_cursor() {
    u16 pos = cursor_y * MAX_COL + cursor_x;
    outb(CURSOR_CTRL, 14);                    //高字节
    outb(CURSOR_DATA, (u8)(pos >> 8));   
    outb(CURSOR_CTRL, 15);                    //低字节
    outb(CURSOR_DATA, (u8)(pos & 0xff));
}

void console_clear() {
    int i;
    for (i = 0; i < 2 * MAX_COL * MAX_ROW; i += 2) {
        vram[i] = ' ';
        vram[i + 1] = WHITE;
    }
    cursor_x = 0;
    cursor_y = 0;
    set_cursor();
}
static void scroll() {
    if (cursor_y >= 25) {
        int i;
        for (i = 1; i < 25; ++i) {
            memcpy(vram + i * 2 * MAX_COL, vram + 2 * (i - 1) * MAX_COL, 2 * MAX_COL);
        }    
        cursor_y = 24;
        // 清空最后一行的信息
        for (i = 0; i < 80; ++i) {
            vram[2 * (MAX_COL * cursor_y + i)] = ' ';
            vram[2 * (MAX_COL * cursor_y + i) + 1] = WHITE;
        }
    }
}

void console_putc(char c, u8 color) {
    if (c == 0x08 && cursor_x) {
          cursor_x--;
    } else if (c == 0x09) {
          cursor_x = (cursor_x+8) & ~(8-1);
    } else if (c == '\r') {
          cursor_x = 0;
    } else if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c >= ' ') {
        vram[2 * (cursor_y * MAX_COL + cursor_x)] = c;
        vram[2 * (cursor_y * MAX_COL + cursor_x) + 1] = color;
        cursor_x++;
    }

    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y ++;
    } else if (cursor_x < 0) {
        if (!cursor_y) cursor_x = 0;
        else {
            cursor_x = 79;
            --cursor_y;
        }
    }
    scroll();
    set_cursor();
}

void console_print(char *s) {
    while (*s) {
        console_putc(*s++, WHITE);
    }
}

void console_print_color(char *s, u8 color) {
    while (*s) {
        console_putc(*s++, color);
    }
}

void console_print_hex(u32 n, u8 color) {
    static char *hex = "0123456789abcdef";
    char s[9];
    int i;
    for (i = 0; i < 8; ++i) {
        s[i] = hex[(n >> ((7 - i) * 4)) & 0x0f];
    }
    s[8] = '\0';
    console_print_color(s, color);
}

void console_print_dec(u32 n, u8 color) {
    int cnt = 0;
    if (!n) {
        console_print_color("0", color);
        return;
    }
    u32 tmp = n;
    while (n) {
        n /= 10;
        cnt++;
    }
    char s[cnt + 1];
    s[cnt] = '\0';
    while (tmp) {
        s[--cnt] = (char)(tmp % 10 + '0');
        tmp /= 10;
    }
    console_print_color(s, color);
}
