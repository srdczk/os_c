//
// Created by srdczk on 20-3-8.
//
#pragma once

#include "types.h"
#include "inode.h"
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


extern dir root_dir;

void open_root_dir(partition *part);

dir *dir_open(partition* part, u32 inode_no);

bool search_dir_entry(partition *part, dir *pdir, const char *name, dir_entry *entry);

void dir_close(dir *d);

void create_dir_entry(char *filename, u32 inode_no, file_type type, dir_entry *entry);

bool sync_dir_entry(dir *parent_dir, dir_entry *entry, void *buf);



