//
// Created by srdczk on 20-3-9.
//
#include "../include/dir.h"
#include "../include/fs.h"
#include "../include/pmm.h"
#include "../include/string.h"
#include "../include/file.h"
#include "../include/stdio.h"

// 根目录
dir root_dir;

void open_root_dir(partition *part) {
    root_dir.node = inode_open(part, part->sb->root_inode_no);
    root_dir.dir_pos = 0;
}

// 打开 inode_no 目录
dir *dir_open(partition* part, u32 inode_no){
    dir *res_dir = (dir *)pmm_malloc(sizeof(dir));
    res_dir->node = inode_open(part, inode_no);
    res_dir->dir_pos = 0;
    return res_dir;
}

// part分区内寻找name 文件或者目录, 并且存入 entry
bool search_dir_entry(partition *part, dir *pdir, const char *name, dir_entry *entry) {
    // 12 个直接块 + 128 间接块
    u32 block_cnt = 140;

    u32 *all_blocks = (u32 *)pmm_malloc((block_cnt) * sizeof(u32));
    if (!all_blocks) {
        kprintf("Can't malloc for blocks\n");
        return 0;
    }

    u32 index = 0;
    // 直接块
    while (index < 12) {
        all_blocks[index] = pdir->node->i_sectors[index];
        index++;
    }

    index = 0;

    if (pdir->node->i_sectors[12]) {
        // 如果有一级间接块, 从硬盘读取
        ide_read_secs(part->devno, pdir->node->i_sectors[12], all_blocks + 12, 1);
    }

    u8 *buf = (u8 *)pmm_malloc(SECTOR_SIZE);
    dir_entry *p_de = (dir_entry *)buf;
    u32 dir_entry_size = part->sb->dir_entry_size;
    u32 dir_entry_cnt = SECTOR_SIZE / dir_entry_size;
    while (index < block_cnt) {
        if (!all_blocks[index]) {
            index++;
            continue;
        }
        // 否则, 从硬盘读取
        ide_read_secs(part->devno, all_blocks[index], buf, 1);
        u32 dir_index = 0;
        while (dir_index < dir_entry_cnt) {
            if (!strcmp(p_de->filename, name)) {
                memcpy(p_de, entry, dir_entry_size);
                pmm_free(buf);
                pmm_free(all_blocks);
                return 1;
            }
            dir_index++;
            p_de++;
        }
        index++;
        p_de = (dir_entry *)buf;
        memset(buf, '\0', SECTOR_SIZE);
    }
    pmm_free(buf);
    pmm_free(all_blocks);
    return 0;
}

// 关闭目录
void dir_close(dir *d) {
    if (d == &root_dir) return;
    inode_close(d->node);
    pmm_free(d);
}

// 内存中初始化 entry
void create_dir_entry(char *filename, u32 inode_no, file_type type, dir_entry *entry) {
    // 把 '\0' 页复制
    memcpy(filename, entry->filename, strlen(filename) + 1);
    entry->i_no = inode_no;
    entry->type = type;
}

// 写入父目录
bool sync_dir_entry(dir *parent_dir, dir_entry *entry, void *buf) {
    inode *dir_inode = parent_dir->node;
    u32 dir_size = dir_inode->i_size;
    u32 dir_entry_size = cur_part->sb->dir_entry_size;

    u32 dir_entrys_per_sec = (SECTOR_SIZE / dir_entry_size);
    int block_lba = -1;

    u8 index = 0;
    u32 all_blocks[140] = {0};

    while (index < 12) {
        all_blocks[index] = dir_inode->i_sectors[index];
        index++;
    }

    dir_entry *dir_e = (dir_entry *)buf;
    int block_bitmap_index = -1;

    index = 0;
    while (index < 140) {
        block_bitmap_index = -1;
        if (!all_blocks[index]) {
            block_lba = block_bitmap_alloc(cur_part);
            if (block_lba == -1) {
                kprintf("alloc fail! block bitmap\n");
                return 0;
            }
            block_bitmap_index = block_lba - cur_part->sb->data_start_lba;
            // 同步 bitmap
            bitmap_sync(cur_part, block_bitmap_index, BLOCK_BITMAP);
            block_bitmap_index = -1;
            // 直接块
            if (index < 12) {
                dir_inode->i_sectors[index] = all_blocks[index] = block_lba;
            } else if (index == 12) {
                dir_inode->i_sectors[index] = block_lba;
                block_lba = -1;
                block_lba = block_bitmap_alloc(cur_part);

                if (block_lba == -1) {
                    block_bitmap_index = dir_inode->i_sectors[index] - cur_part->sb->data_start_lba;
                    bitmap_set(&cur_part->block_bitmap, block_bitmap_index, 0);
                    dir_inode->i_sectors[index] = 0;
                    kprintf("lloc block bitmap for sync_dir_entry fail!\n");
                    return 0;
                }

                block_bitmap_index = block_lba - cur_part->sb->data_start_lba;

                bitmap_sync(cur_part, block_bitmap_index, BLOCK_BITMAP);

                all_blocks[index] = block_lba;
                ide_write_secs(cur_part->devno, dir_inode->i_sectors[index], all_blocks + index, 1);
            } else {
                // 一级间接块
                all_blocks[index] = block_lba;
                ide_write_secs(cur_part->devno, dir_inode->i_sectors[12], all_blocks + 12, 1);
            }
            memset(buf, '\0', SECTOR_SIZE);
            memcpy(entry, buf, dir_entry_size);
            ide_write_secs(cur_part->devno, all_blocks[index], buf, 1);
            dir_inode->i_size += dir_entry_size;
            return 1;
        }
        ide_read_secs(cur_part->devno, all_blocks[index], buf, 1);
        u8 dir_entry_index = 0;
        while (dir_entry_index < dir_entrys_per_sec) {
            if ((dir_e + dir_entry_index)->type == UNKNOWN) {
                memcpy(entry, dir_e + dir_entry_index, dir_entry_size);
                ide_write_secs(cur_part->devno, all_blocks[index], buf, 1);
                dir_inode->i_size += dir_entry_size;
                return 1;
            }
            dir_entry_index++;
        }
        index++;
    }
    kprintf("directory full!\n");
    return 0;
}

// 删除目录
bool delete_dir_entry(partition *part, dir *pdir, u32 inode_no, void *io_buf) {
    inode *dir_inode = pdir->node;
    u32 block_index = 0;
    u32 all_blocks[140] = {0};
    while (block_index < 12) {
        all_blocks[block_index] = dir_inode->i_sectors[block_index];
        block_index++;
    }
    if (dir_inode->i_sectors[12]) {
        ide_read_secs(part->devno, dir_inode->i_sectors[12], all_blocks + 12, 1);
    }

    // 目录项存储时, 不会跨越扇区
    u32 dir_entry_size = part->sb->dir_entry_size;
    u32 dir_entrys_per_sec = (SECTOR_SIZE / dir_entry_size);
    dir_entry *dir_e = (dir_entry *)io_buf;
    dir_entry *dir_entry_found = NULL;
    u8 dir_entry_index;
    u8 dir_entry_cnt;
    bool is_dir_first_block = 0;

    block_index = 0;
    while (block_index < 140) {
        is_dir_first_block = 0;
        if (!all_blocks[block_index]) {
            block_index++;
            continue;
        }
        dir_entry_index = dir_entry_cnt = 0;
        memset(io_buf, '\0', SECTOR_SIZE);
        // 获得目录项
        ide_read_secs(part->devno, all_blocks[block_index], io_buf, 1);
        // 遍历目录项
        while (dir_entry_index < dir_entrys_per_sec) {
            if ((dir_e + dir_entry_index)->type != UNKNOWN) {
                if (!strcmp((dir_e + dir_entry_index)->filename, ".")) {
                    is_dir_first_block = 1;
                } else if (strcmp((dir_e + dir_entry_index)->filename, ".") &&strcmp((dir_e + dir_entry_index)->filename, "..")) {
                    dir_entry_cnt++;
                    if ((dir_e + dir_entry_index)->i_no == inode_no) {
                        dir_entry_found = dir_e + dir_entry_index;
                    }
                }
            }
            dir_entry_index++;
        }
        if (!dir_entry_found) {
            block_index++;
            continue;
        }
        if (dir_entry_cnt == 1 && !is_dir_first_block) {
            u32 block_bitmap_index = all_blocks[block_index] - part->sb->data_start_lba;
            bitmap_set(&part->block_bitmap, block_bitmap_index, 0);
            bitmap_sync(part, block_bitmap_index, BLOCK_BITMAP);

            if (block_index < 12) {
                dir_inode->i_sectors[block_index] = 0;
            } else {
                u32 indirect_blocks = 0;
                u32 indirect_block_index = 12;
                while (indirect_block_index < 140) {
                    if (all_blocks[indirect_block_index]) indirect_block_index++;
                }

                if (indirect_blocks > 1) {
                    all_blocks[block_index] = 0;
                    ide_write_secs(part->devno, dir_inode->i_sectors[12], all_blocks + 12, 1);
                } else {
                    block_bitmap_index = dir_inode->i_sectors[12] - part->sb->data_start_lba;
                    bitmap_set(&part->block_bitmap, block_bitmap_index, 0);
                    bitmap_sync(part, block_bitmap_index, BLOCK_BITMAP);
                    dir_inode->i_sectors[12] = 0;
                }
            }
        } else {
            memset(dir_entry_found, '\0', dir_entry_size);
            ide_write_secs(part->devno, all_blocks[block_index], io_buf, 1);
        }
        dir_inode->i_size -= dir_entry_size;
        memset(io_buf, '\0', 2 * SECTOR_SIZE);
        inode_sync(part, dir_inode, io_buf);

        return 1;
    }
    return 0;
}



