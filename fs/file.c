//
// Created by srdczk on 20-3-9.
//
#include "../include/file.h"
#include "../include/fs.h"
#include "../include/thread.h"
#include "../include/isr.h"
#include "../include/bitmap.h"
#include "../include/console.h"
#include "../include/string.h"

file file_table[MAX_FILE];

void table_init() {
    int i;
    for (i = 0; i < MAX_FILE; ++i) {
        file_table[i].fd_inode = NULL;
    }
}

// file_table 获取空闲位
int get_free_in_ftable() {
    u16 index = 3;
    // 0 1 2固定
    while (index < MAX_FILE) {
        if (!file_table[index].fd_inode) return index;
        index++;
    }
    // 没有空闲的
    return -1;
}

int pcb_fd_install(u32 global_index) {
    // 全局描述副安装到 pcb 的fd 中
    task_struct *cur = running_thread();
    u8 index = 3;
    while (index < MAX_OPEN_FILE) {
        if (cur->fd_table[index] == -1) break;
        index++;
    }
    if (index == MAX_OPEN_FILE) return -1;
    cur->fd_table[index] = global_index;
    return index;
}

// bitmap 分配节点
int inode_bitmap_alloc(partition *part) {
    // 分配一个
    int index = bitmap_apply(&part->inode_bitmap, 1);
    if (index == -1) return -1;
    bitmap_set(&part->inode_bitmap, index, 1);
    return index;
}

int block_bitmap_alloc(partition *part) {
    // 分配一个
    int index = bitmap_apply(&part->block_bitmap, 1);
    if (index == -1) return -1;
    bitmap_set(&part->block_bitmap, index, 1);
    return index + part->sb->data_start_lba;
}

// 同步到硬盘
void bitmap_sync(partition *part, u32 index, bitmap_type type) {

    u32 off_sec = index / PAGE_SIZE;

    u32 off_size = off_sec * BLOCK_SIZE;

    u32 sec_lba;
    char *bitmap_off;

    if (type == INODE_BITMAP) {
        sec_lba = part->sb->inode_bitmap_lba + off_sec;
        bitmap_off = part->inode_bitmap.map + off_size;
    } else {
        sec_lba = part->sb->block_bitmap_lba + off_sec;
        bitmap_off = part->block_bitmap.map + off_size;
    }
    ide_write_secs(part->devno, sec_lba, bitmap_off, 1);
}

// 创建 文件, 成功返回文件描述符, 否则返回 -1
int file_create(dir *parent_dir, char *filename, u8 flag) {
    void *buf = pmm_malloc(2 * SECTOR_SIZE);
    if (!buf) {
        kprintf("malloc io buf error in file_create!\n");
        return -1;
    }
    // 操作失败的回滚操作
    u8 rollback_step = 0;
    // inode 创建
    u32 inode_no = inode_bitmap_alloc(cur_part);
    if (inode_no == -1) {
        kprintf("file_create inode alloc error\n");
        return -1;
    }
    inode *new_file_inode = (inode *)pmm_malloc(sizeof(inode));
    if (!new_file_inode) {
        kprintf("file_create inode alloc error\n");
        rollback_step = 1;
        // 回滚操作
        goto rollback;
    }
    // inode 初始化
    inode_init(inode_no, new_file_inode);

    int index = get_free_in_ftable();
    if (index == -1) {
        kprintf("max open files error\n");
        rollback_step = 2;
        goto rollback;
    }
    file_table[index].fd_inode = new_file_inode;
    file_table[index].fd_pos = 0;
    file_table[index].fd_flag = flag;
    file_table[index].fd_inode->write_deny = 0;

    dir_entry entry;
    memset(&entry, '\0', sizeof(dir_entry));

    create_dir_entry(filename, inode_no, REGULAR, &entry);

    if (!sync_dir_entry(parent_dir, &entry, buf)) {
        kprintf("sync dir_entry to disk failed\n");
        rollback_step = 3;
        goto rollback;
    }

    memset(buf, '\0', 1024);
    inode_sync(cur_part, parent_dir->node, buf);
    memset(buf, '\0', 1024);
    // 新创建的文件内容写回到硬盘
    inode_sync(cur_part, new_file_inode, buf);
    // inode bitmap 写回

    bitmap_sync(cur_part, inode_no, INODE_BITMAP);

    list_add_last(&cur_part->open_inodes, &new_file_inode->inode_tag);
    new_file_inode->i_open_cnts = 1;
    pmm_free(buf);
    return pcb_fd_install(index);

    rollback:
    switch (rollback_step) {
        case 3:
            // 沿着下面做, 不需要break
            memset(&file_table[index], '\0', sizeof(file));
        case 2:
            pmm_free(new_file_inode);
        case 1:
            bitmap_set(&cur_part->inode_bitmap, inode_no, 0);
            break;
    }
    pmm_free(buf);
    return -1;
}

int file_open(u32 inode_no, u8 flag) {
    int index = get_free_in_ftable();
    if (index == -1) {
        kprintf("max open file\n");
        return -1;
    }
    file_table[index].fd_inode = inode_open(cur_part, inode_no);
    file_table[index].fd_pos = 0;
    file_table[index].fd_flag = flag;
    bool *write = &file_table[index].fd_inode->write_deny;

    if ((flag & O_WRONLY) || (flag & O_RDWR)) {
        // 文件能够写入
        // 关中断
        int_status istatus = disable_int();
        if (!(*write)) {
            *write = 1;
            set_int_status(istatus);
        } else {
            set_int_status(istatus);
            kprintf("readonly file!\n");
            return -1;
        }
    }
    return pcb_fd_install(index);
}

int file_close(file *f) {
    if (!f) return -1;
    f->fd_inode->write_deny = 0;
    inode_close(f->fd_inode);
    f->fd_inode = NULL;
    return 0;
}

// 文件写入 ->
int file_write(file *f, const void *buf, u32 cnt) {
    // 最大支持 512 * 140 字节
    if ((f->fd_inode->i_size + cnt) > (SECTOR_SIZE * 140)) {
        kprintf("too large file\n");
        return -1;
    }

    // 申请
    u8 *io_buf = pmm_malloc(SECTOR_SIZE);
    if (!io_buf) {
        kprintf("file_write: io_buff alloc fail\n");
        return -1;
    }
    // 记录文件块
    u32 *all_blocks = (u32 *)pmm_malloc(SECTOR_SIZE + 48);
    if (!all_blocks) {
        kprintf("file_write: all_blocks alloc fail\n");
        return -1;
    }
    const u8 *src = buf;
    // 已经写入的数据
    u32 bytes_written = 0;
    // 剩余的数据
    u32 size_left = cnt;
    int block_lba = -1;
    // 记录块 位图 的index
    u32 block_bitmap_index = 0;
    u32 sec_index;
    u32 sec_lba;
    u32 sec_off_bytes;
    u32 sec_left_bytes;
    u32 chunk_size;
    // 获取一级间接表
    int indirect_block_table;
    u32 block_index;
    // 如果是第一次写入
    if (!f->fd_inode->i_sectors[0]) {
        block_lba = block_bitmap_alloc(cur_part);
        if (block_lba == -1) {
            kprintf("file_write: block_bitmap_alloc fail\n");
            return -1;
        }
        f->fd_inode->i_sectors[0] = block_lba;
        kprintf("\nBLOCK_LBA:%x\n", block_lba);
        block_bitmap_index = block_lba - cur_part->sb->data_start_lba;
        bitmap_sync(cur_part, block_bitmap_index, BLOCK_BITMAP);
    }
    u32 file_has_used_blocks = f->fd_inode->i_size / BLOCK_SIZE + 1;
    u32 file_will_use_blocks = (f->fd_inode->i_size + cnt) / BLOCK_SIZE + 1;
    u32 add_blocks = file_will_use_blocks - file_has_used_blocks;
    // 如果不需要新的扇区
    if (!add_blocks) {
        if (file_has_used_blocks <= 12) {
            block_index = file_has_used_blocks - 1;
            all_blocks[block_index] = f->fd_inode->i_sectors[block_index];
        } else {
            indirect_block_table = f->fd_inode->i_sectors[12];
            ide_read_secs(cur_part->devno, indirect_block_table, all_blocks + 12, 1);
        }
    } else {
        // 需要申请新的扇区
        if (file_will_use_blocks <= 12) {
            block_index = file_has_used_blocks - 1;
            all_blocks[block_index] = f->fd_inode->i_sectors[block_index];
            block_index = file_has_used_blocks;
            while (block_index < file_will_use_blocks) {
                block_lba = block_bitmap_alloc(cur_part);
                if (block_lba == -1) {
                    kprintf("file_write: block_bitmap_alloc fail\n");
                    return -1;
                }
                f->fd_inode->i_sectors[block_index] = all_blocks[block_index] = block_lba;
                // 同步到硬盘
                block_bitmap_index = block_lba - cur_part->sb->data_start_lba;
                bitmap_sync(cur_part, block_bitmap_index, BLOCK_BITMAP);
                block_index++;
            }
        } else if (file_has_used_blocks <= 12 && file_will_use_blocks > 12) {
            block_index = file_has_used_blocks - 1;
            all_blocks[block_index] = f->fd_inode->i_sectors[block_index];

            block_lba = block_bitmap_alloc(cur_part);

            if (block_lba == -1) {
                kprintf("file_write: block_alloc 2 fail\n");
                return -1;
            }
            indirect_block_table = block_lba;
            f->fd_inode->i_sectors[12] = block_lba;
            block_index = file_has_used_blocks;
            while (block_index < file_will_use_blocks) {
                block_lba = block_bitmap_alloc(cur_part);
                if (block_lba == -1) {
                    kprintf("file write: block bitmap alloc fail\n");
                    return -1;
                }
                if (block_index < 12) {
                    f->fd_inode->i_sectors[block_index] = block_lba;
                    all_blocks[block_index] = block_lba;
                } else {
                    all_blocks[block_index] = block_lba;
                }
                block_bitmap_index = block_lba - cur_part->sb->data_start_lba;
                bitmap_sync(cur_part, block_bitmap_index, BLOCK_BITMAP);
                block_index++;
            }
//            console_print_color("DEBUG1\n", MAGENTA);
            ide_write_secs(cur_part->devno, indirect_block_table, all_blocks + 12, 1);
        } else if (file_has_used_blocks > 12) {
            //  新旧 数据都在一级间接块中
            indirect_block_table = f->fd_inode->i_sectors[12];
            // 读入
            ide_read_secs(cur_part->devno, indirect_block_table, all_blocks + 12, 1);
            block_index = file_has_used_blocks;
            while (block_index < file_will_use_blocks) {
                block_lba = block_bitmap_alloc(cur_part);
                if (block_lba == -1) {
                    kprintf("alloc bitmap fail\n");
                    return -1;
                }
                all_blocks[block_index++] = block_lba;
                block_bitmap_index = block_lba - cur_part->sb->data_start_lba;

                bitmap_sync(cur_part, block_bitmap_index, BLOCK_BITMAP);
            }
        }
    }
    bool first_write_block = 1;

    f->fd_pos = f->fd_inode->i_size - 1;
    while (bytes_written < cnt) {
        memset(io_buf, '\0', SECTOR_SIZE);
        sec_index = f->fd_inode->i_size / SECTOR_SIZE;
        sec_lba = all_blocks[sec_index];
        sec_off_bytes = f->fd_inode->i_size % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;

        chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;
        if (first_write_block) {
            // 如果是第一次读取
            ide_read_secs(cur_part->devno, sec_lba, io_buf, 1);
            first_write_block = 0;
        }
        memcpy(src, io_buf + sec_off_bytes, chunk_size);
//        console_print_color("DEBUG2\n", GREEN);
        ide_write_secs(cur_part->devno, sec_lba, io_buf, 1);

        src += chunk_size;
        f->fd_inode->i_size += chunk_size;
        f->fd_pos += chunk_size;
        bytes_written += chunk_size;
        size_left -= chunk_size;
    }
    inode_sync(cur_part, f->fd_inode, io_buf);
    pmm_free(all_blocks);
    pmm_free(io_buf);
    return bytes_written;
}


int file_read(file* f, void *buf, u32 cnt) {
    u8 *buf_des = (u8 *)buf;
    u32 size = cnt;
    u32 size_left = size;

    if ((f->fd_pos + cnt) > f->fd_inode->i_size) {
        size = f->fd_inode->i_size - f->fd_pos;
        size_left = size;
        // 已经读到文件末尾
        if (!size) return -1;
    }

    u8 *io_buf = pmm_malloc(BLOCK_SIZE);

    if (!io_buf) {
        kprintf("file_read: malloc for io_buf fail\n");
        return -1;
    }

    u32 *all_blocks = (u32 *)pmm_malloc(BLOCK_SIZE + 48);
    if (!all_blocks) {
        kprintf("file_read all_blocks alloc fail!\n");
        return -1;
    }

    // 数据块起始, 终止的位置
    u32 block_read_start_index = f->fd_pos / BLOCK_SIZE;
    u32 block_read_end_index = (f->fd_pos + size) / BLOCK_SIZE;
    u32 read_blocks = block_read_start_index - block_read_end_index;

    u32 indirect_block_table;
    u32 block_index;
    // 不需要跨越扇区
    if (!read_blocks) {
        // 读取直接块
        if (block_read_end_index < 12) {
            block_index = block_read_end_index;
            all_blocks[block_index] = f->fd_inode->i_sectors[block_index];
        } else {
            // 间接块, 读取中间
            indirect_block_table = f->fd_inode->i_sectors[12];

            ide_read_secs(cur_part->devno, indirect_block_table, all_blocks + 12, 1);
        }
    } else {
        // 要读取多个块
        if(block_read_end_index < 12){
            block_index = block_read_start_index;
            while (block_index <= block_read_end_index) {
                all_blocks[block_index] = f->fd_inode->i_sectors[block_index];
                block_index++;
            }
        } else if (block_read_start_index < 12 && block_read_end_index >= 12) {
            block_index = block_read_start_index;
            while (block_index < 12) {
                all_blocks[block_index] = f->fd_inode->i_sectors[block_index];
                block_index++;
            }
            indirect_block_table = f->fd_inode->i_sectors[12];
            ide_read_secs(cur_part->devno, indirect_block_table, all_blocks + 12, 1);
        } else {
            indirect_block_table = f->fd_inode->i_sectors[12];
            ide_read_secs(cur_part->devno, indirect_block_table, all_blocks + 12, 1);
        }
    }

    u32 sec_index;
    u32 sec_lba;
    u32 sec_off_bytes;
    u32 sec_left_bytes;
    u32 chunk_size;
    u32 bytes_read = 0;
    while (bytes_read < size) {
        sec_index = f->fd_pos / BLOCK_SIZE;
        sec_lba = all_blocks[sec_index];
        sec_off_bytes= f->fd_pos % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;
        chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;
        memset(io_buf, '\0', BLOCK_SIZE);
        ide_read_secs(cur_part->devno, sec_lba, io_buf, 1);
        memcpy(io_buf + sec_off_bytes, buf_des, chunk_size);
        buf_des += chunk_size;
        f->fd_pos += chunk_size;
        bytes_read += chunk_size;
        size_left -= chunk_size;
    }
    pmm_free(all_blocks);
    pmm_free(io_buf);
    return bytes_read;
}







