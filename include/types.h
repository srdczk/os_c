#pragma once

typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;
typedef unsigned char u8;
typedef char s8;

typedef struct {
    u16 limit;
    u32 base;
}__attribute__((packed))descriptor;
