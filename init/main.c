#include "../include/console.h"


void main() {
    console_clear();
    console_print("NIMSILE\n");
    console_print_color("NIMASOILE\n", RED);
    console_print_color("KKKOKK\n", GREEN);
    console_print_hex(0xabcdef01, GREEN);
    console_print("\n");
    console_print_dec(3782373, RED);
    while (1);
}
