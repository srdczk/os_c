//
// Created by srdczk on 20-3-8.
//
#pragma once

#include "types.h"


// 超级块, 大小512byte, 一个扇区
typedef struct {
    // 文件系统类型
    u32 magic;
    // 分区扇区数量
    u32 sec_num;
    // inode 数量
    u32 inode_num;
    // 分区 起始扇区
    u32 part_lba_base;
    // 块位图 起始扇区, 占用扇区
    u32 block_bitmap_lba;
    u32 block_bitmap_secs;
    // inode 位图
    u32 inode_bitmap_lba;
    u32 inode_bitmap_secs;
    // inode 起始扇区, 占用扇区
    u32 inode_table_lba;
    u32 inode_table_secs;
    u32 data_start_lba;
    u32 root_inode_no;
    u32 dir_entry_size;
    // 占满一个扇区
    u8 pad[460];
} __attribute__ ((packed)) super_block;