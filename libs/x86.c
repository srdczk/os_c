#include "../include/x86.h"

u8 inb(u16 port){
    u8 data;
    asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

void outb(u16 port, u8 data) {
    asm volatile ("outb %b0, %w1" : : "a" (data), "Nd" (port));
}


void outsw(u16 port, const void* addr, u32 word_cnt) {
    asm volatile ("cld; rep outsw" : "+S" (addr), "+c" (word_cnt) : "d" (port));
}

void insw(u16 port, void* addr, u32 word_cnt){
    asm volatile ("cld; rep insw" : "+D" (addr), "+c" (word_cnt) : "d" (port) : "memory");
}

void outw(u16 port, u16 data) {
    asm volatile ("outw %0, %1" :: "a" (data), "d" (port));
}

void insl(u32 port, void *addr, int cnt) {
    asm volatile (
    "cld;"
    "repne; insl;"
    : "=D" (addr), "=c" (cnt)
    : "d" (port), "0" (addr), "1" (cnt)
    : "memory", "cc");
}

void outsl(u32 port, void *addr, int cnt) {
    asm volatile (
    "cld;"
    "repne; outsl;"
    : "=S" (addr), "=c" (cnt)
    : "d" (port), "0" (addr), "1" (cnt)
    : "memory", "cc");
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

u32 read_esp() {
    u32 esp;
    asm volatile ("movl %%esp, %0" : "=r"(esp));
    return esp;
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
