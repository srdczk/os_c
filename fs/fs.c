//
// Created by srdczk on 20-3-8.
//
#include "../include/fs.h"
#include "../include/file.h"
#include "../include/stdio.h"
#include "../include/thread.h"
#include "../include/string.h"
#include "../include/pmm.h"
#include "../include/debug.h"
#include "../include/console.h"

// 默认操作分区
partition *cur_part;

// 挂载分区, 选择操作分区
static bool mount_partition(list_node *node, void *arg) {
    char *name = (char *)arg;
    partition *part = node2entry(partition, part_tag, node);
    if (!strcmp(part->name, name)) {
        cur_part = part;
        u16 devno = part->devno;
        // 读入超级块
        super_block *sb = (super_block *)pmm_malloc(SECTOR_SIZE);
        // 内存中加载分区信息
        cur_part->sb = (super_block *)pmm_malloc(sizeof(super_block));
        if (!cur_part->sb) PANIC("alloc fail!");

        memset(sb, '\0', SECTOR_SIZE);
        ide_read_secs(devno, cur_part->start_lba + 1, sb, 1);

        memcpy(sb, cur_part->sb, sizeof(super_block));
        cur_part->block_bitmap.map = (char *)pmm_malloc(sb->block_bitmap_secs * SECTOR_SIZE);
        if (!cur_part->block_bitmap.map) PANIC("alloc fail! -> bitmap");
        cur_part->block_bitmap.map_len = sb->block_bitmap_secs * SECTOR_SIZE;
        ide_read_secs(devno, sb->block_bitmap_lba, cur_part->block_bitmap.map, sb->block_bitmap_secs);

        cur_part->inode_bitmap.map = (char *)pmm_malloc(sb->inode_bitmap_secs * SECTOR_SIZE);
        if (!cur_part->inode_bitmap.map) PANIC("malloc fail -> inode bm");
        cur_part->inode_bitmap.map_len = sb->inode_bitmap_secs * SECTOR_SIZE;

        ide_read_secs(devno, sb->inode_bitmap_lba, cur_part->inode_bitmap.map, sb->inode_bitmap_secs);

        list_init(&cur_part->open_inodes);

        kprintf("%s mount done!\n", part->name);
        return 1;
    }
    // 继续
    return 0;
}

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

// 解析路径
static char *path_parse(char *pathname, char *name_store) {
    if (pathname[0] == '/') {
        // 跳过 ‘/’
        while (*(++pathname) == '/');
    }

    // 最上层 路径
    while (*pathname != '/' && *pathname) {
        *name_store++ = *pathname++;
    }

    if (!pathname[0]) return NULL;

    return pathname;
}


// 返回路径 层数 /x/y/z
int path_depth(char *pathname) {
    char *p = pathname;
    char name[MAX_FILE_NAME_LEN];
    u32 depth = 0;
    p = path_parse(p, name);
    while (name[0]) {
        depth++;
        memset(name, '\0', MAX_FILE_NAME_LEN);
        if (p) {
            p = path_parse(p, name);
        }
    }
    return depth;
}

// 搜索文件
static int search_file(const char* pathname, path_search_record *record) {
    // 如果查找的是根目录
    if (!strcmp(pathname, "/") || !strcmp(pathname, "/.") || !strcmp(pathname, "/..")) {
        record->parent_dir = &root_dir;
        record->type = DIRECTORY;
        record->searched_path[0] = 0;
        return 0;
    }
    u32 path_len = strlen(pathname);
    char *sub_path = (char *)pathname;
    dir *parent_dir = &root_dir;
    dir_entry entry;
    char name[MAX_FILE_NAME_LEN] = {0};
    record->parent_dir = parent_dir;
    record->type = UNKNOWN;

    u32 parent_inode_no = 0;
    sub_path = path_parse(sub_path, name);
    while (name[0]) {
        strcat("/", record->searched_path);
        strcat(name, record->searched_path);
        if (search_dir_entry(cur_part, parent_dir, name, &entry)) {
            memset(name, '\0', MAX_FILE_NAME_LEN);
            if (sub_path) sub_path = path_parse(sub_path, name);
            if (entry.type == DIRECTORY) {
                parent_inode_no = parent_dir->node->i_no;
                dir_close(parent_dir);
                parent_dir = dir_open(cur_part, entry.i_no);
                record->parent_dir = parent_dir;
                continue;
            } else if (entry.type == REGULAR) {
                record->type = REGULAR;
                return entry.i_no;
            }
            // 未找到
        } else return -1;
    }

    // 遍历完整路径
    dir_close(record->parent_dir);

    record->parent_dir = dir_open(cur_part, parent_inode_no);
    record->type = DIRECTORY;

    return entry.i_no;
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
    // 默认分区, -> 主分区
    char default_part[8] = "sda1";
    list_traversal(&part_list, mount_partition, (void *)default_part);

    // 创建根目录
    open_root_dir(cur_part);

    table_init();
}

/* 打开或创建文件成功后,返回文件描述符,否则返回-1 */
int fs_open(const char *pathname, u8 flag) {
    if (pathname[strlen(pathname) - 1] == '/') {
        kprintf("can't open dir: %s\n", pathname);
        return -1;
    }
    int fd = -1;
    path_search_record record;
    memset(&record, '\0', sizeof(path_search_record));
    u32 depth = path_depth((char *)pathname);
    int inode_no = search_file(pathname, &record);
    bool found = inode_no == -1 ? 0 : 1;
    if (record.type == DIRECTORY) {
        kprintf("can't open dir: %s\n", pathname);
        dir_close(record.parent_dir);
        return -1;
    }
    u32 search_depth = path_depth(record.searched_path);
    if (search_depth != depth) {
        kprintf("can't access:%s\n", pathname);
        dir_close(record.parent_dir);
        return -1;
    }

    if (!found && !(flag & O_CREAT)) {
        //未找到并且不是 创建
        kprintf("in path %s not found: %s\n", record.searched_path, (strrchr(record.searched_path, '/') + 1));
        dir_close(record.parent_dir);
        return -1;
    } else if (found && (flag & O_CREAT)) {
        kprintf("%s already exist\n", pathname);
        dir_close(record.parent_dir);
        flag &= ~O_CREAT;
    }
    switch (flag & O_CREAT) {
        case O_CREAT:
            kprintf("creating file...\n");
            fd = file_create(record.parent_dir, (strrchr(pathname, '/') + 1), flag);
            dir_close(record.parent_dir);
            break;
            // 否则直接打开文件
        default:
            fd = file_open(inode_no, flag);
            break;
    }
    return fd;
}

// 转换成全局的描述符
static u32 fd_local2global(u32 local_fd) {
    task_struct *cur = running_thread();
    return (u32)cur->fd_table[local_fd];
}

int fs_close(u32 fd) {
    int res = -1;
    if (fd > 2) {
        res = file_close(&file_table[fd_local2global(fd)]);
        running_thread()->fd_table[fd] = -1;
    }
    return res;
}

int fs_write(u32 fd, const void *buf, u32 cnt) {
    if (fd == stdout_no) {
        char tmp_buf[1024];
        memcpy(buf, tmp_buf, cnt);
        tmp_buf[cnt] = '\0';
        console_print(tmp_buf);
        return cnt;
    }
    file *wf = &file_table[fd_local2global(fd)];
    if ((wf->fd_flag & O_WRONLY) || (wf->fd_flag & O_RDWR)) {
        u32 res = file_write(wf, buf, cnt);
        return res;
    } else {
        console_print("File flag not WRONLY or RDWR \n");
        return -1;
    }
}

int fs_read(u32 fd, void *buf, u32 cnt) {
    return file_read(&file_table[fd_local2global(fd)], buf, cnt);
}

int fs_lseek(u32 fd, int offset, u8 seek) {
    u32 gfd = fd_local2global(fd);
    file *f = &file_table[gfd];
    int new_pos = 0;
    int file_size = (int)f->fd_inode->i_size;
    switch (seek) {
        // 设置
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = (int)f->fd_pos + offset;
            break;
        case SEEK_END:
            new_pos = file_size + offset;
            break;
    }
    if (new_pos < 0 || new_pos > (file_size - 1)) return -1;
    f->fd_pos = new_pos;
    return new_pos;
}

int fs_unlink(const char *pathname) {
    path_search_record record;
    memset(&record, '\0', sizeof(path_search_record));
    int inode_no = search_file(pathname, &record);
    if (inode_no == -1) {
        kprintf("file %s not found\n", pathname);
        dir_close(record.parent_dir);
        return -1;
    }
    if (record.type == DIRECTORY) {
        kprintf("can't delete a directory\n");
        dir_close(record.parent_dir);
        return -1;
    }
    u32 file_index = 0;
    while (file_index < MAX_FILE) {
        if (file_table[file_index].fd_inode && (u32)inode_no == file_table[file_index].fd_inode->i_no) {
            break;
        }
        file_index++;
    }
    if (file_index < MAX_FILE) {
        dir_close(record.parent_dir);
        kprintf("file is in user\n");
        return -1;
    }

    void *io_buf = pmm_malloc(SECTOR_SIZE + SECTOR_SIZE);

    if (!io_buf) {
        dir_close(record.parent_dir);
        kprintf("can't alloc io_buf: fs_unlink\n");
        return -1;
    }

    dir *parent_dir = record.parent_dir;
    delete_dir_entry(cur_part, parent_dir, inode_no, io_buf);
    inode_release(cur_part, inode_no);
    pmm_free(io_buf);
    dir_close(record.parent_dir);
    // 删除成功
    return 0;
}

// 创建目录
int fs_mkdir(const char *pathname) {
    u8 rollback_step = 0;
    void *io_buf = pmm_malloc(2 * SECTOR_SIZE);
    if (!io_buf) {
        kprintf("fs_mkdir: pmm_malloc for io_buf fail\n");
        return -1;
    }

    path_search_record record;
    memset(&record, '\0', sizeof(path_search_record));
    int inode_no = -1;
    if ((inode_no = search_file(pathname, &record)) != -1) {
        kprintf("pmm_mkdir: file or directory %s exist!\n", pathname);
        rollback_step = 1;
        goto rollback;
    } else {
        u32 depth = path_depth((char *)pathname);
        u32 search_depth = path_depth(record.searched_path);
        if (depth != search_depth) {
            kprintf("pmm_mkdir: fail for path\n");
            rollback_step = 1;
            goto rollback;
        }
    }

    dir *parent_dir = record.parent_dir;

    char *dirname = strrchr(record.searched_path, '/') + 1;

    inode_no = inode_bitmap_alloc(cur_part);

    if ((inode_no = inode_bitmap_alloc(cur_part)) == -1) {
        kprintf("alloc inode fail\n");
        rollback_step = 1;
        goto rollback;
    }

    inode new_dir_inode;
    inode_init(inode_no, &new_dir_inode);

    u32 block_bitmap_index = 0;
    int block_lba = -1;
    block_lba = block_bitmap_alloc(cur_part);
    if (block_lba == -1) {
        kprintf("block bitmap alloc for create directory\n");
        rollback_step = 2;
        goto rollback;
    }
    new_dir_inode.i_sectors[0] = block_lba;
    // 同步到硬盘上
    block_bitmap_index = block_lba - cur_part->sb->data_start_lba;
    bitmap_sync(cur_part, block_bitmap_index, BLOCK_BITMAP);
    memset(io_buf, '\0', 2 * SECTOR_SIZE);

    dir_entry *entry = (dir_entry *)io_buf;

    memcpy(".", entry->filename, 1);
    entry->i_no = inode_no;
    entry->type = DIRECTORY;

    entry++;
    memcpy("..", entry->filename, 2);
    entry->i_no = parent_dir->node->i_no;
    entry->type = DIRECTORY;

    ide_write_secs(cur_part->devno, new_dir_inode.i_sectors[0], io_buf, 1);
    new_dir_inode.i_size = 2 * cur_part->sb->dir_entry_size;

    dir_entry new_dir_entry;
    memset(&new_dir_entry, '\0', sizeof(dir_entry));
    create_dir_entry(dirname, inode_no, DIRECTORY, &new_dir_entry);
    memset(io_buf, '\0', 2 * SECTOR_SIZE);
    if (!sync_dir_entry(parent_dir, &new_dir_entry, io_buf)) {
        kprintf("pmm: sync_direntry fail\n");
        rollback_step = 2;
        goto rollback;
    }

    memset(io_buf, '\0', 2 * SECTOR_SIZE);

    inode_sync(cur_part, parent_dir->node, io_buf);
    memset(io_buf, '\0', 2 * SECTOR_SIZE);
    inode_sync(cur_part, &new_dir_inode, io_buf);

    bitmap_sync(cur_part, inode_no, INODE_BITMAP);

    pmm_free(io_buf);

    dir_close(record.parent_dir);

    return 0;
    // 错误回滚
    rollback:
    switch (rollback_step) {
        case 2:
            bitmap_set(&cur_part->inode_bitmap, inode_no, 0);
        case 1:
            dir_close(record.parent_dir);
            break;
    }
    pmm_free(io_buf);
    return -1;
}

// 打开目录
dir *fs_opendir(const char *name) {
    if (name[0] == '/' && (!name[1] || name[0] == '.')) {
        return &root_dir;
    }
    path_search_record record;
    memset(&record, 0, sizeof(path_search_record));
    int inode_no = search_file(name, &record);
    dir *res = NULL;
    if (inode_no == -1) {
        kprintf("In %s, sub path %s not exist\n", name, record.searched_path);
    } else {
        if (record.type == REGULAR) {
            kprintf("%s is regular file!\n", name);
        } else if (record.type == DIRECTORY) {
            res = dir_open(cur_part, inode_no);
        }
    }
    dir_close(record.parent_dir);
    return res;
}

//关闭目录
int fs_closedir(dir *d) {
    int res = -1;
    if (d) {
        dir_close(d);
        res = 0;
    }
    return res;
}

dir_entry *fs_readdir(dir *d) {
    return dir_read(d);
}

void fs_rewinddir(dir *d) {
    d->dir_pos = 0;
}

int fs_rmdir(const char *pathname) {
    path_search_record record;
    memset(&record, '\0', sizeof(path_search_record));
    int inode_no = search_file(pathname, &record);
    int res = -1;
    if (inode_no == -1) kprintf("%s not exist\n", pathname);
    else {
        if (record.type == REGULAR) {
            kprintf("%s is regular file!\n", pathname);
        } else {
            dir *d = dir_open(cur_part, inode_no);
            if (!dir_is_empty(d)) {
                // 不能删除非空目录
                kprintf("dir %s is not empty!\n", pathname);
            } else {
                if (!dir_remove(record.parent_dir, d)) {
                    res = 0;
                }
            }
            dir_close(d);
        }
    }
    dir_close(record.parent_dir);
    return res;
}

static u32 get_parent_dir_inode_nr(u32 child_inode_nr, void *io_buf) {
    inode *child_dir_inode = inode_open(cur_part, child_inode_nr);
    u32 block_lba = child_dir_inode->i_sectors[0];
    inode_close(child_dir_inode);
    ide_read_secs(cur_part->devno, block_lba, io_buf, 1);
    dir_entry *entry = (dir_entry *)io_buf;
    // 0 -> . , 1 -> ..
    return entry[1].i_no;
}

static int get_child_dir_name(u32 p_inode_nr, u32 c_inode_nr, char *path, void *io_buf) {
    inode *parent_dir_inode = inode_open(cur_part, p_inode_nr);
    u8 block_index = 0;
    u32 all_blocks[140] = {0};
    u32 block_cnt = 12;
    while (block_index < 12) {
        all_blocks[block_index] = parent_dir_inode->i_sectors[block_index];
        block_index++;
    }
    if (parent_dir_inode->i_sectors[12]) {
        ide_read_secs(cur_part->devno, parent_dir_inode->i_sectors[12], all_blocks + 12, 1);
        block_cnt = 140;
    }
    inode_close(parent_dir_inode);
    dir_entry *entry = (dir_entry *)io_buf;
    u32 dir_entry_size = cur_part->sb->dir_entry_size;
    u32 dir_entrys_per_sec = (512 / dir_entry_size);
    block_index = 0;
    while (block_index < block_cnt) {
        if (all_blocks[block_index]) {
            ide_read_secs(cur_part->devno, all_blocks[block_index], io_buf, 1);
            u8 dir_entry_index = 0;
            while (dir_entry_index < dir_entrys_per_sec) {
                if ((entry + dir_entry_index)->i_no == c_inode_nr) {
                    strcat("/", path);
                    strcat((entry + dir_entry_index)->filename, path);
                    return 0;
                }
                dir_entry_index++;
            }
        }
        block_index++;
    }
    return -1;
}

char *fs_getcwd(char *buf, u32 size) {
    void *io_buf = pmm_malloc(SECTOR_SIZE);
    if (!io_buf) return NULL;

    task_struct *cur = running_thread();
    int parent_inode_nr = 0;
    int child_inode_nr = cur->cwd_inode_nr;
    // 如果是根目录
    if (!child_inode_nr) {
        buf[0] = '/';
        buf[1] = '\0';
        return buf;
    }

    memset(buf, '\0', size);

    char full_path[MAX_PATH_LEN] = {0};

    while (child_inode_nr) {
        // 知道找到根目录
        parent_inode_nr = get_parent_dir_inode_nr(child_inode_nr, io_buf);
        if (get_child_dir_name(parent_inode_nr, child_inode_nr, full_path, io_buf) == -1) {
            pmm_free(io_buf);
            return NULL;
        }
        child_inode_nr = parent_inode_nr;
    }
    char *last_slash;
    while ((last_slash = strrchr(full_path, '/'))) {
        u16 len = strlen(buf);
        strcpy(last_slash, buf + len);
        *last_slash = '\0';
    }
    pmm_free(io_buf);
    return buf;
}

// 更换工作目录
int fs_chdir(const char *path) {
    int res = -1;
    path_search_record record;
    memset(&record, '\0', sizeof(path_search_record));
    int inode_no = search_file(path, &record);
    if (inode_no != -1) {
        if (record.type == DIRECTORY) {
            running_thread()->cwd_inode_nr = inode_no;
            res = 0;
        } else {
            kprintf("can't change to a regular file!\n");
        }
    }
    dir_close(record.parent_dir);
    return res;
}
// 文件属性
int fs_stat(const char *path, stat *buf) {
    // 如果是根目录
    if (!strcmp(path, "/") || !strcmp(path, "/.") || !strcmp(path, "/..")) {
        buf->st_ino = 0;
        buf->st_type = DIRECTORY;
        buf->st_size = root_dir.node->i_size;
        return 0;
    }
    int res = -1;
    path_search_record record;
    memset(&record, '\0', sizeof(path_search_record));

    int inode_no = search_file(path, &record);
    if (inode_no != -1) {
        inode *obj_inode = inode_open(cur_part, inode_no);
        buf->st_size = obj_inode->i_size;
        inode_close(obj_inode);
        buf->st_type = record.type;
        buf->st_ino = inode_no;
        res = 0;
    } else {
        kprintf("fs_stat: %s not found\n", path);
    }
    dir_close(record.parent_dir);
    return res;
}


