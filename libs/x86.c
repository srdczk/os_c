#include "../include/x86.h"

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
void lidt(descriptor *desc) {
    asm volatile ("lidt (%0)" :: "r" (desc));
}

u32 read_ebp() {
    u32 ebp;
    asm volatile ("movl %%ebp, %0" : "=r" (ebp));
    return ebp;
}

u32 read_eip(void) {
    u32 eip;
    asm volatile("movl 4(%%ebp), %0" : "=r" (eip));
    return eip;
}



u32 read_eflags() {
    u32 eflags;
    asm volatile ("pushfl \n\t"
                  "popl %0"
                  :
                  "=g"(eflags));
    return eflags;
}
