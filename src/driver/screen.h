#include "port.h"

#define CURSOR_DATA 0x3d5
#define CURSOR_CTRL 0x3d4

#define MAX_ROW 25
#define MAX_COL 80

void kprint(char *msg);
int print_at(char x, int cursor, char color);
int get_cursor_offset();
void set_cursor_offset(int cursor);
int get_offset(int r, int c);
void clear_screen();
void clear_last_line();
int get_row(int offset);
int get_col(int offset);
void memcpy(char *src, char *des, int len);

