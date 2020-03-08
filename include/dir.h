//
// Created by srdczk on 20-3-8.
//
#pragma once

#include "types.h"
#include "inode.h"
#include "fs.h"
//目录项描述

// 最长文件名
#define MAX_FILE_NAME_LEN 16
// 目录结构
typedef struct {
    inode *node;
    // 目录内偏移
    u32 dir_pos;
    // 数据缓存
    u8 dir_buf[512];
} dir;

// 目录项
typedef struct {
    char filename[MAX_FILE_NAME_LEN];
    // 对应的inode 编号
    u32 i_no;
    //文件类型
    file_type type;
} dir_entry;

