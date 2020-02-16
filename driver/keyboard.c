#include "keyboard.h"

void print_letter(u8 code) {
    kprint("DEBUG ");
    char s[3];
    int_to_string(code, s);
    kprint(s);
    kprint("\n");
}
