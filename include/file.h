//
// Created by srdczk on 20-3-9.
//
#pragma once

#include "types.h"
#include "inode.h"
#include "dir.h"

// 文件的结构
typedef struct {
    u32 fd_pos;
    u32 fd_flag;
    inode *fd_inode;
} file;
// stdin, stdout, stderr
typedef enum {
    stdin_no,
    stdout_no,
    stderr_no
} std_fd;
// 位图类型
typedef enum {
    INODE_BITMAP,
    BLOCK_BITMAP
} bitmap_type;

// 最大打开文件数
#define MAX_FILE 0x20

extern file file_table[];

void table_init();

int get_free_in_ftable();

int pcb_fd_install(u32 global_index);

int inode_bitmap_alloc(partition *part);

int block_bitmap_alloc(partition *part);

void bitmap_sync(partition *part, u32 index, bitmap_type type);

int file_create(dir *parent_dir, char *filename, u8 flag);

int file_close(file *f);

int file_write(file *f, const void *buf, u32 cnt);

int file_read(file* f, void *buf, u32 cnt);