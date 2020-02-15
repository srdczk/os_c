#include "x86.h"

u8 inb(u16 port) {
    u8 data;
    asm volatile ("inb %1, %0" : "=a" (data) : "d" (port));
    return data;
}

void outb(u16 port, u8 data) {
    asm volatile ("outb %0, %1" :: "a" (data), "d" (port));
}
void outw(u16 port, u16 data) {
    asm volatile ("outw %0, %1" :: "a" (data), "d" (port));
}
void sti() {
    asm volatile("sti");
}
void cli() {
    asm volatile("cli");
}
void lidt(desc *pd) {
    asm volatile ("lidt (%0)" :: "r" (pd));
}
