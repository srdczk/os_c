//
// Created by srdczk on 20-3-16.
//

#pragma once
#include "types.h"

// elf 文件头
typedef struct {
    u8 e_elf[16];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u32 e_entry;
    u32 e_phoff;
    u32 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
} elfhdr;

// 程序头表
typedef struct {
    u32 p_type;
    u32 p_offset;
    u32 p_va;
    u32 p_pa;
    u32 p_filesz;
    u32 p_memsz; u32
    p_flags;
    u32 p_align;
} proghdr;

// 段类型
typedef enum {
    PT_NULL, // 忽略
    PT_LOAD, // 可加载程序段
    PT_DYNAMIC, // 动态加载
    PT_INTERP,  // 动态加载名称
    PT_NOTE, // 一些辅助信息
    PT_SHLIB, // 保留
    PT_PHDR // 程序头表
} segment_type;

int kernel_exec(const char *name, const char **av);

