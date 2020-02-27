//
// Created by chengzekun on 20-2-26.
//
#include "../include/debug.h"
#include "../include/console.h"
#include "../include/isr.h"

void panic(char *filename, int line, const char *func, char *cond) {
    disable_int();

    console_print_color("\n!error!\n", RED);
    console_print_color("filename:", RED);console_print_color(filename, RED);console_print("\n");
    console_print_color("line:0x", RED);console_print_hex(line, RED);console_print("\n");
    console_print_color("function:", RED);console_print_color(func, RED);console_print("\n");
    console_print_color("condition:", RED);console_print_color(cond, RED);
    while (1);

}

