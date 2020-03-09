#pragma once

#include "types.h"
#include "list.h"
#include "bitmap.h"
#include "super_block.h"

typedef struct {
    u16 base, ctrl;
} channel;

// 分区结构体
typedef struct {
    // 起始扇区
    u32 start_lba;
    // 扇区数
    u32 sec_num;
    // 所属的硬盘编号
    u16 devno;
    // 存储在 partition 链表中
    list_node part_tag;
    char name[8];
    // 分区的超级块
    super_block *sb;
    // 块位图
    bitmap block_bitmap;
    // inode 位图
    bitmap inode_bitmap;
    // inode 链表
    list open_inodes;
} partition;

typedef struct {
    u8 valid; // 判断是否存在
    u32 sets;
    u32 size;
    // 名称 ->
    u8 model[41];
    // 主分区
    partition prim_parts[4];
    // 逻辑分区
    partition logic_parts[8];
} ide_device;

extern list part_list;

extern ide_device ide_devices[];

void ide_init();

bool ide_device_valid(u16 ideno);

u32 ide_device_size(u16 ideno);

int ide_read_secs(u16 ideno, u32 secno, void *dst, u32 nsecs);

int ide_write_secs(u16 ideno, u32 secno, void *src, u32 nsecs);