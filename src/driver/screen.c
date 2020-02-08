#include "screen.h"
#include "port.h"

// 光标位置
int get_cursor_offset();

void set_cursor_offset(int offset);

int print_char(char c, int col, int row, char attr);

int get_offset(int c, int r);

int get_offset_row(int offset);

int get_offset_col(int offset);

void kprint_at(char *msg, int c, int r) {
    int offset;
    if (c >= 0 && r >= 0) offset = get_offset(c, r);
    else {
        offset = get_cursor_offset();
        r = get_offset_row(offset);
        c = get_offset_col(offset);
    }
    int i = 0;
    while (msg[i] != 0) {
        offset = print_char(msg[i++], c, r, WHITE_ON_BLACK);
        r = get_offset_row(offset);
        c = get_offset_col(offset);
    }
}

void kprint(char *msg) {
    kprint_at(msg, -1, -1);
}

int print_char(char c, int col, int row, char attr) {
    char *vm = (char *)VIDEO_ADDRESS;
    if (!attr) attr = WHITE_ON_BLACK;
    if (col >= MAX_COLS || row >= MAX_ROWS) {
        vm[2 * (MAX_COLS) * (MAX_ROWS) - 2] = 'E';
        vm[2 * (MAX_COLS) * (MAX_ROWS) - 1] = RED_ON_WHITE;
        return get_offset(col, row);
    }

    int offset;
    if (col >= 0 && row >= 0) offset = get_offset(col, row);
    else offset = get_cursor_offset();

    if (c == '\n') {
        row = get_offset_row(offset);
        // row 行, col  列
        offset = get_offset(0, row + 1);
    } else {
        vm[offset] = c;
        vm[offset + 1] = attr;
        offset += 2;
    }
    set_cursor_offset(offset);
    return offset;
}

int get_cursor_offset() {
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8;
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return 2 * offset;
}

void set_cursor_offset(int offset) {
    offset /= 2;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset & 0xff));
}

void clear_screen() {
    int screen_size = MAX_COLS * MAX_ROWS;
    int i = 0;
    char *vm = (char *)VIDEO_ADDRESS;
    for (i = 0; i < screen_size; ++i) {
        vm[2 * i] = ' ';
        vm[2 * i + 1] = WHITE_ON_BLACK;
    }

    set_cursor_offset(get_offset(0, 0));
}

int get_offset(int c, int r) { return 2 * (r * MAX_COLS + c);  }
int get_offset_row(int offset) { return offset / (2 * MAX_COLS); }
int get_offset_col(int offset) { return (offset - 2 * get_offset_row(offset) * MAX_COLS)/ 2; }
