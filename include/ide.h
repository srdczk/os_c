//
// Created by srdczk on 20-3-7.
//
#pragma once

#include "types.h"
#include "sync.h"
#include "bitmap.h"
#include "list.h"


// (c语言允许 未定义的struct)
// 分区结构
typedef struct partition {
    //起始扇区
    u32 start_lba;
    //扇区数
    u32 sec_cnt;
    //属于哪个硬盘
    struct disk* my_disk;
    list_node part_tag;
    char name[8];
    //超级块
    struct super_block* sb;
    //块位图
    bitmap block_bitmap;
    // inode 位图
    bitmap inode_bitmap;
    //inode 链表
    list open_inodes;
} partition;

// 硬盘的结构
typedef struct disk{
    char name[8];
    // 通道
    struct ide_channel* my_channel;
    // 标记硬盘号 -> 从 0 开始
    u8 dev_no;
    //主分区
    struct partition prim_parts[4];
    //逻辑分区
    struct partition logic_parts[8];
} disk;

// 通道结构
typedef struct ide_channel {
    char name[8];
    //起始端口号
    u16 port_base;
    //所用中断号
    u8 irq_no;
    semaphore sema, semb;
    // 等待中断
    u8 expecting_intr;
    //通道上的硬盘
    struct disk devices[2];
} ide_channel;

extern u8 channel_num;
extern ide_channel channels[2];

void ide1_init();

void ide_read(disk* hd, u32 lba, void* buf, u32 sec_cnt);

void ide_write(disk* hd, u32 lba, void* buf, u32 sec_cnt);

void hd_handler(u8 irq_no);

void ide_init();
