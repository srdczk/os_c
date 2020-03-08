//
// Created by srdczk on 20-3-8.
//
#pragma once
#include "types.h"
#include "list.h"
#include "ide.h"

// inode 结构
typedef struct {
    // 编号
    u32 i_no;
    // 普通文件(->文件大小), 目录项-> 目录项大小之和
    u32 i_size;
    // 文件打开次数
    u32 i_open_cnts;
    // 避免同时写入
    bool write_deny;
    // 0-11 直接块, 1个一级间接块
    u32 i_sectors[13];
    list_node inode_tag;
} inode;

