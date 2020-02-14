#pragma once
// 定义
typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long s64;
typedef unsigned long long u64;
typedef struct {
    u16 limit;
    u32 base;
}__attribute__((packed))desc;

typedef struct {
    u32 ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, useresp, ss;
} trapframe;

u8 inb(u16 port);
void outb(u16 port, u8 data);
void outw(u16 port, u16 data);
void lidt(desc *pd);
// 不使用内存对齐

