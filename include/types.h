#pragma once

#define NULL 0

#define EFLAGS_MBS (1 << 1)
#define EFLAGS_IF_1 (1 << 9)
#define EFLAGS_IF_0 (0)
#define EFLAGS_IOPL_3 (3 << 12)
#define EFLAGS_IOPL_0 (0)


typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;
typedef unsigned char u8;
typedef char s8;

typedef struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) descriptor;

