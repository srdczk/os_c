//
// Created by srdczk on 20-3-9.
//
#include "../include/inode.h"
#include "../include/file.h"
#include "../include/thread.h"
#include "../include/string.h"
#include "../include/isr.h"

// 存储 inode 位置
typedef struct {
    // 是否 跨越扇区
    bool two_sec;
    // 扇区编号
    u32 sec_lba;
    // 偏移量
    u32 off_size;
} inode_pos;

// 获取 inode 扇区 和 偏移量
static void inode_locate(partition *part, u32 inode_no, inode_pos *pos) {
    u32 inode_table_lba = part->sb->inode_table_lba;
    u32 inode_size = sizeof(inode);
    // inode_no 节点相对偏移
    u32 off_size = inode_no * inode_size;
    u32 off_sec = off_size / SECTOR_SIZE;
    // 起始地址
    u32 off_size_in_sec = off_size % SECTOR_SIZE;

    u32 left_in_sec = SECTOR_SIZE - off_size_in_sec;
    // 如果 放不下
    if (left_in_sec < inode_size) pos->two_sec = 1;
    else pos->two_sec = 0;

    pos->sec_lba = inode_table_lba + off_sec;
    pos->off_size = off_size_in_sec;

}

// 将 inode 写入 分区
void inode_sync(partition *part, inode *node, void *buf) {
    u8 inode_no = node->i_no;
    inode_pos pos;
    // 信息存入 inode_pos
    inode_locate(part, inode_no, &pos);
    // 硬盘中的 inode 结构不需要 inode_tag 和 i_open_cnt
    inode pure_inode;
    memcpy(node, &pure_inode, sizeof(inode));
    // 清除存储在硬盘中 无用的
    pure_inode.i_open_cnts = 0;
    pure_inode.write_deny = 0;
    pure_inode.inode_tag.pre = pure_inode.inode_tag.next = NULL;

    char *inode_buf = (char *)buf;
    // 跨越扇区
    if (pos.two_sec) {
        ide_read_secs(part->devno, pos.sec_lba, inode_buf, 2);
        memcpy(&pure_inode, inode_buf + pos.off_size, sizeof(inode));
        ide_write_secs(part->devno, pos.sec_lba, inode_buf, 2);
    } else {
        ide_read_secs(part->devno, pos.sec_lba, inode_buf, 1);
        memcpy(&pure_inode, inode_buf + pos.off_size, sizeof(inode));
        ide_write_secs(part->devno, pos.sec_lba, inode_buf, 1);
    }
}

// inode_no 返回相应的inode
inode *inode_open(partition *part, u32 inode_no) {
    list_node *node = part->open_inodes.head.next;
    inode *res_inode;
    while (node != &part->open_inodes.tail) {
        res_inode = node2entry(inode, inode_tag, node);
        if (res_inode->i_no == inode_no) {
            res_inode->i_open_cnts++;
            return res_inode;
        }
        node = node->next;
    }

    // 链表中没有找到, 硬盘读入并加入链表
    inode_pos pos;
    // 加载到 pos 中
    inode_locate(part, inode_no, &pos);

    task_struct *cur = running_thread();

    u32 pgdir = cur->pgdir;
    // 置空页表( 在malloc申请时候作为内核空间 )
    cur->pgdir = 0;
    res_inode = (inode *)pmm_malloc(sizeof(inode));
    cur->pgdir = pgdir;

    char *inode_buf;
    // 跨越扇区
    if (pos.two_sec) {
        inode_buf = (char *)pmm_malloc(2 * SECTOR_SIZE);
        ide_read_secs(part->devno, pos.sec_lba, inode_buf, 2);
    } else {
        inode_buf = (char *)pmm_malloc(SECTOR_SIZE);
        ide_read_secs(part->devno, pos.sec_lba, inode_buf, 1);
    }


    memcpy(inode_buf + pos.off_size, res_inode, sizeof(inode));

    list_add_last(&part->open_inodes, &res_inode->inode_tag);
    res_inode->i_open_cnts = 1;

    pmm_free(inode_buf);
    return res_inode;
}

// 关闭 inode, 或 --open_cnt
void inode_close(inode *node) {
    int_status istatus = disable_int();
    // 保证 原子
    if (!--node->i_open_cnts) {
        list_remove(&node->inode_tag);
        task_struct *cur = running_thread();
        u32 pgdir = cur->pgdir;
        cur->pgdir = 0;
        pmm_free(node);
        cur->pgdir = pgdir;
    }
    set_int_status(istatus);
}


// 初始化 inode
void inode_init(u32 inode_no, inode *new_inode) {
    new_inode->i_no = inode_no;
    new_inode->i_size = 0;
    new_inode->i_open_cnts = 0;
    new_inode->write_deny = 0;
    // 初始化索引
    memset((char *)new_inode->i_sectors, '\0', 13 * sizeof(u32));
}

// 回收 inode
void inode_release(partition  *part, u32 inode_no) {
    inode *inode_to_del = inode_open(part, inode_no);

    u8 block_index = 0;
    u8 block_cnt = 12;
    u32 block_bitmap_index;
    u32 all_blocks[140] = {0};

    while (block_index < 12) {
        all_blocks[block_index] = inode_to_del->i_sectors[block_index];
        block_index++;
    }

    if (inode_to_del->i_sectors[12]) {
        ide_read_secs(part->devno, inode_to_del->i_sectors[12], all_blocks + 12, 1);
        block_cnt = 140;

        block_bitmap_index = inode_to_del->i_sectors[12] - part->sb->data_start_lba;
        bitmap_set(&part->block_bitmap, block_bitmap_index, 0);
        bitmap_sync(part, block_bitmap_index, BLOCK_BITMAP);
    }

    block_index = 0;
    while (block_index < block_cnt) {
        if (all_blocks[block_index]) {
            block_bitmap_index = all_blocks[block_index] - part->sb->data_start_lba;
            bitmap_set(&part->block_bitmap, block_bitmap_index, 0);
            bitmap_sync(part, block_index, BLOCK_BITMAP);
        }
        block_index++;
    }
    bitmap_set(&part->inode_bitmap, inode_no, 0);
    bitmap_sync(part, inode_no, INODE_BITMAP);
    inode_close(inode_to_del);
}
