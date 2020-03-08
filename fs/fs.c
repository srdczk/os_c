//
// Created by srdczk on 20-3-8.
//
#include "../include/fs.h"
#include "../include/ide.h"
#include "../include/dir.h"
#include "../include/stdio.h"
#include "../include/string.h"
#include "../include/pmm.h"
#include "../include/console.h"

// 初始化 分区信息
static void partition_format(partition *part) {
    // 初始化 块位图
    // 引导扇区和超级块占用的扇区
    u32 boot_sector_secs = 1;
    u32 super_block_secs = 1;
    // inode 位图, 数组占用的扇区
    u32 inode_bitmap_secs = DIV_ROUND_UP(MAX_FILES_PER_PART, BITS_PER_SECTOR);
    u32 inode_table_secs = DIV_ROUND_UP(((sizeof(inode) * MAX_FILES_PER_PART)), SECTOR_SIZE);
    // 已经使用的扇区
    u32 used_secs = boot_sector_secs + super_block_secs + inode_bitmap_secs + inode_table_secs;
    // 剩余扇区
    u32 free_secs = part->sec_num - used_secs;
    // 空闲扇区位图占用扇区
    u32 block_bitmap_secs = DIV_ROUND_UP(free_secs, BITS_PER_SECTOR);
    // 继续计算剩余
    u32 block_bitmap_bit_len = free_secs - block_bitmap_secs;
    // 最终的空闲扇区位图占用扇区
    block_bitmap_secs = DIV_ROUND_UP(block_bitmap_bit_len, BITS_PER_SECTOR);
    // 初始化超级块
    super_block sb;
    // magic
    sb.magic = 0x20200308;
    // 分区扇区数量, inode, 起始扇区
    sb.sec_num = part->sec_num;
    sb.inode_num = MAX_FILES_PER_PART;
    sb.part_lba_base = part->start_lba;
    //块位图起始扇区第三块
    sb.block_bitmap_lba = sb.part_lba_base + 2;
    sb.block_bitmap_secs = block_bitmap_secs;
    // inode 位图
    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_secs;
    sb.inode_bitmap_secs = inode_bitmap_secs;
    // inode 数组
    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_secs;
    sb.inode_table_secs = inode_table_secs;
    // 剩余部分起始
    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_secs;
    // 根目录 root 为 0
    sb.root_inode_no = 0;
    sb.dir_entry_size = sizeof(dir_entry);

    kprintf("%s info:\n", part->name);

    kprintf("   magic:0x%x\n", sb.magic);
    kprintf("   part_lba_base:0x%x\n", sb.part_lba_base);
    kprintf("   all_sectors:0x%x\n", sb.sec_num);
    kprintf("   inode_num:0x%x\n", sb.inode_num);
    kprintf("   block_bitmap_lba:0x%x\n", sb.block_bitmap_lba);
    kprintf("   block_bitmap_secs:0x%x\n", sb.block_bitmap_secs);
    kprintf("   inode_bitmap_lba:0x%x\n", sb.inode_bitmap_lba);
    kprintf("   inode_bitmap_secs:0x%x\n", sb.inode_bitmap_secs);
    kprintf("   inode_table_lba:0x%x\n", sb.inode_table_lba);
    kprintf("   inode_table_secs:0x%x\n", sb.inode_table_secs);
    kprintf("   data_start_lba:0x%x\n", sb.data_start_lba);
    kprintf("   root_inode_no:0x%x\n", sb.root_inode_no);
    kprintf("   dir_entry_size:0x%x\n", sb.dir_entry_size);
    // 超级块 写入硬盘
    u16 devno = part->devno;
    ide_write_secs(devno, part->start_lba + 1, &sb, 1);
    kprintf("   super_block_lba:0x%x\n", part->start_lba + 1);
    // 寻找占用内存最大的, 申请堆内存
    u32 buf_size = (sb.block_bitmap_secs >= sb.inode_bitmap_secs ? sb.block_bitmap_secs : sb.inode_bitmap_secs);
    buf_size = (buf_size >= sb.inode_table_secs ? buf_size : sb.inode_table_secs) * SECTOR_SIZE;
    u8 *buf = (u8 *)pmm_malloc(buf_size);
    // 根目录占用
    buf[0] |= 0x01;
    // 空闲块 占字节长度
    u32 block_bitmap_last_byte = block_bitmap_bit_len / 8;
    u8 block_bitmap_last_bit  = block_bitmap_bit_len % 8;
    // last_size 最后一个扇区
    u32 last_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE);
    memset(&buf[block_bitmap_last_byte], 0xff, last_size);
    u8 bit_index = 0;
    while (bit_index <= block_bitmap_last_bit) {
        buf[block_bitmap_last_byte] &= ~(1 << bit_index++);
    }
    // 写入块位图扇区
    ide_write_secs(devno, sb.block_bitmap_lba, buf, sb.block_bitmap_secs);
    // inode 位图扇区
    memset(buf, '\0', buf_size);
    buf[0] |= 0x1;
    ide_write_secs(devno, sb.inode_bitmap_lba, buf, sb.inode_bitmap_secs);
    // inode 表扇区
    memset(buf, '\0', buf_size);  // 清空
    inode* i = (inode *)buf;
    // 本目录和父目录
    i->i_size = sb.dir_entry_size * 2;
    // 根目录占用 0 inode
    i->i_no = 0;
    i->i_sectors[0] = sb.data_start_lba;
    ide_write_secs(devno, sb.inode_table_lba, buf, sb.inode_table_secs);
    // data 扇区
    memset(buf, '\0', buf_size);
    dir_entry *p_de = (dir_entry *)buf;
    memcpy(".", p_de->filename, 1);
    p_de->i_no = 0;
    p_de->type = DIRECTORY;
    p_de++;

    memcpy("..", p_de->filename, 2);
    p_de->i_no = 0;
    p_de->type = DIRECTORY;
    ide_write_secs(devno, sb.data_start_lba, buf, 1);

    kprintf("   root_dir_lba:0x%x\n", sb.data_start_lba);
    kprintf("   %s format done\n", part->name);
    pmm_free(buf);
}

// 初始化文件系统
void filesys_init() {
    // 暂时 只 处理 0 号磁盘
    u16 devno = 0;
    partition *part = ide_devices[devno].prim_parts;
    super_block *sb = (super_block *)pmm_malloc(SECTOR_SIZE);
    console_print_color("SHIDE\n", GREEN);
    // 申请 内存空间 存储超级块信息
    u32 cnt = 0;
    while (cnt < 12) {
        // 4 个主 分区, 8 逻辑分区
        if (cnt == 4) part = ide_devices[devno].logic_parts;
        // 如果分区存在
        if (part->sec_num) {
            memset(sb, '\0', SECTOR_SIZE);
            // 根据魔数, 判断是否已经创建了文件系统
            ide_read_secs(devno, part->start_lba + 1, sb, 1);
            if (sb->magic == 0x20200308) {
                kprintf("%s has fileSystem!\n", part->name);
            } else {
                kprintf("formatting %s...\n", part->name);
                partition_format(part);
            }
        }
        part++;
        cnt++;
    }
    pmm_free(sb);
}


