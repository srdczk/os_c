//
// Created by srdczk on 20-3-8.
//
#pragma once

#include "types.h"

// 分区最大文件
#define MAX_FILES_PER_PART 4096
// 扇区 bit
#define BITS_PER_SECTOR 4096
#define SECTOR_SIZE 512
#define BLOCK_SIZE SECTOR_SIZE

// 文件类型枚举
typedef enum {
    UNKNOWN,
    REGULAR,
    DIRECTORY
}  file_type;

void filesys_init();