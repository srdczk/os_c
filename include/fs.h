//
// Created by srdczk on 20-3-8.
//
#pragma once

#include "types.h"
#include "dir.h"
#include "ide.h"

#define MAX_PATH_LEN 512

// 文件查找过程中的记录
typedef struct {
    char searched_path[MAX_PATH_LEN];
    dir *parent_dir;
    file_type type;
} path_search_record;

extern partition *cur_part;

int path_depth(char *pathname);

void filesys_init();

int fs_open(const char *pathname, u8 flag);

int file_open(u32 inode_no, u8 flag);

int fs_close(u32 fd);

int fs_write(u32 fd, const void *buf, u32 cnt);

int fs_read(u32 fd, void *buf, u32 cnt);

int fs_lseek(u32 fd, int offset, u8 seek);

int fs_unlink(const char *pathname);

int fs_mkdir(const char *pathname);

dir *fs_opendir(const char *name);

int fs_closedir(dir *d);

dir_entry *fs_readdir(dir *d);

void fs_rewinddir(dir *d);

int fs_rmdir(const char *pathname);

char *fs_getcwd(char *buf, u32 size);

int fs_chdir(const char *path);
// 文件属性
int fs_stat(const char *path, stat *buf);