#include "screen.h"

void kprint(char *msg) {
    int i = 0;
    int cursor = get_cursor_offset();
    while (msg[i]) {
        cursor = print_at(msg[i++], cursor, 0x0f);
    }
}

int print_at(char x, int cursor, char color) {
    char *vga = (char *)0xb8000;
    int i;
    while (cursor >= 2 * MAX_ROW * MAX_COL) {
        // 1 - 25 行全部复制到前面去, 清空 最后一行
        for (i = 1; i < MAX_ROW; ++i) {
            memcpy(vga + 2 * (i * MAX_COL), vga + 2 * ((i - 1) * MAX_COL), 2 * MAX_COL);
        }
        clear_last_line();
        cursor -= 2 * MAX_COL;
    }
    int r = get_row(cursor), c = get_col(cursor);
    vga[cursor] = x == '\n' ? ' ' : x;
    vga[cursor + 1] = color;
    if (x == '\n') {
        if (r == MAX_ROW - 1) {
            for (i = 1; i < MAX_ROW; ++i) {
                memcpy(vga + 2 * (i * MAX_COL), vga + 2 * ((i - 1) * MAX_COL), 2 * MAX_COL);
            }
            clear_last_line();
            set_cursor_offset(get_offset(MAX_ROW - 1, c));
            return get_offset(MAX_ROW - 1, c);
        } else {
            set_cursor_offset(get_offset(r + 1, c));
            return get_offset(r + 1, c);
        }
    } else {
        set_cursor_offset(cursor + 2);
        return cursor + 2;
    }
}

int get_cursor_offset() {
    port_byte_out(CURSOR_CTRL, 14);
    int offset = port_byte_in(CURSOR_DATA) << 8;
    port_byte_out(CURSOR_CTRL, 15);
    offset += port_byte_in(CURSOR_DATA);
    return 2 * offset;
}

void set_cursor_offset(int offset) {
    offset /= 2;
    port_byte_out(CURSOR_CTRL, 14);
    port_byte_out(CURSOR_DATA, (unsigned char)(offset >> 8));
    port_byte_out(CURSOR_CTRL, 15);
    port_byte_out(CURSOR_DATA, (unsigned char)(offset & 0xff));
}

int get_offset(int r, int c) {
    return 2 * (MAX_COL * r + c);
}
void clear_screen() {
    int i;
    char *vga = (char *)0xb8000;
    for (i = 0; i < 2 * MAX_COL * MAX_ROW; i += 2) {
        vga[i] = ' ';
        vga[i + 1] = 0x0f;
    }
    set_cursor_offset(0);
}
void clear_last_line() {
    char *begin = (char *)(0xb8000 + 2 * (MAX_COL * (MAX_ROW - 1)));
    int i;
    for (i = 0; i < MAX_COL * 2; i += 2) {
        begin[i] = ' ';
        begin[i + 1] = 0x0f;
    }
}
int get_row(int offset) {
    return (offset / 2) / MAX_COL;
}
int get_col(int offset) {
    return (offset - get_row(offset) * 2 * MAX_COL) / 2;
}

void memcpy(char *src, char *des, int len) {
    int i;
    for (i = 0; i < len; ++i) {
        *(des + i) = *(src + i);
    }
}

