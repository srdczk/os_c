//
// Created by srdczk on 20-3-7.
//

#include "../include/ide.h"
#include "../include/debug.h"
#include "../include/irq.h"
#include "../include/stdio.h"
#include "../include/x86.h"

// 硬盘各寄存器的端口号
#define reg_data(channel)	 (channel->port_base + 0)
#define reg_error(channel)	 (channel->port_base + 1)
#define reg_sect_cnt(channel)	 (channel->port_base + 2)
#define reg_lba_l(channel)	 (channel->port_base + 3)
#define reg_lba_m(channel)	 (channel->port_base + 4)
#define reg_lba_h(channel)	 (channel->port_base + 5)
#define reg_dev(channel)	 (channel->port_base + 6)
#define reg_status(channel)	 (channel->port_base + 7)
#define reg_cmd(channel)	 (reg_status(channel))
#define reg_alt_status(channel)  (channel->port_base + 0x206)
#define reg_ctl(channel)	 reg_alt_status(channel)


// 硬盘忙
#define BIT_STAT_BSY	 (1 << 7)
//驱动器就绪
#define BIT_STAT_DRDY	 (1 << 6)
//数据就绪
#define BIT_STAT_DRQ	 (1 << 3)

#define BIT_DEV_MBS	0xa0
#define BIT_DEV_LBA	0x40
#define BIT_DEV_DEV	0x10

#define CMD_IDENTIFY	   0xec
// 读扇区
#define CMD_READ_SECTOR	   0x20
// 写扇区
#define CMD_WRITE_SECTOR   0x30
// 80MB 硬盘, 最大扇区
#define max_lba ((80*1024*1024/512) - 1)
// 通道数量 (一个通道两个硬盘)
u8 channel_num;
ide_channel channels[2];
// 总拓展分区的起始lba
u32 ext_lba_base = 0;
u8 p_no = 0, l_no = 0;
// 分区链表
list partition_list;

#define IRQ_IDE1 0x2e

//区表结构体
typedef struct {
    u8  bootable;
    // 磁头号,起始扇区和起始柱面
    u8  start_head;
    u8  start_sec;
    u8  start_chs;
    //分区类型
    u8  fs_type;
    // 磁头号，结束扇区，结束柱面
    u8  end_head;
    u8  end_sec;
    u8  end_chs;
    //起始扇区lba
    u32 start_lba;
    //扇区数目
    u32 sec_cnt;
} __attribute__ ((packed)) partition_table_entry;

//引导扇区
typedef struct {
    //代码部分
    u8  other[446];
    //分区表项
    partition_table_entry partition_table[4];
    //结束标志 0x55aa(小端0xaa55)
    u16 signature;
} __attribute__ ((packed)) boot_sector;

void ide1_init() {
    irq_enable(IRQ_IDE1);
}

// 写入
static void select_disk(disk* hd) {
    u8 reg = BIT_DEV_LBA | BIT_DEV_MBS;
    if (!hd->dev_no) reg |= BIT_DEV_DEV;
    outb(reg_dev(hd->my_channel), reg);
}
// 写入起始扇区地址和读写扇区数
static void select_sector(disk* hd, u32 lba, u8 sec_cnt) {
    ide_channel* channel = hd->my_channel;

    outb(reg_sect_cnt(channel), sec_cnt);

    outb(reg_lba_l(channel), lba);
    outb(reg_lba_m(channel), (lba >> 8));
    outb(reg_lba_h(channel), (lba >> 16));

    outb(reg_dev(channel), BIT_DEV_MBS | BIT_DEV_LBA | (!hd->dev_no ? BIT_DEV_DEV : 0) | lba >> 24);
}

// 向channel 发送命令
static void cmd_out(ide_channel* channel, u8 cmd) {
    channel->expecting_intr = 1;
    outb(reg_cmd(channel), cmd);
}

// 读入 sec_cnt 个扇区的数据
static void read_from_sector(disk* hd, void* buf, u8 sec_cnt) {
    u32 size;
    if (!sec_cnt) size = 256 * 512;
    else size = sec_cnt * 512;
    insw(reg_data(hd->my_channel), buf, size / 2);
}

// 写入硬盘
static void write_to_sector(disk* hd, void* buf, u8 sec_cnt) {
    u32 size;
    if (!sec_cnt) size = 256 * 512;
    else size = sec_cnt * 512;
    outsw(reg_data(hd->my_channel), buf, size / 2);
}

// 等待 30s , 确保数据传输完成
static bool busy_wait(disk *hd) {
    ide_channel *channel = hd->my_channel;
    s16 time = 30 * 1000;
    while ((time -= 10) >= 0) {
        kprintf("%x\n", reg_alt_status(channel));
        u8 res = inb(reg_status(channel));
        kprintf("%d\n", res);
        if (!(res & BIT_STAT_BSY)) return (res & BIT_STAT_DRQ);
        else mil_sleep(10);
    }
    return 0;
}

// 硬盘读取 sec_cnt 个扇区
void ide_read(disk* hd, u32 lba, void* buf, u32 sec_cnt) {
    // 上锁
    sem_down(&hd->my_channel->sema);

    select_disk(hd);
    // 要转移的扇区数
    u32 secs_op;
    // 完成的扇区数
    u32 secs_done = 0;
    while (secs_done < sec_cnt) {
        if (secs_done + 256 <= sec_cnt) secs_op = 256;
        else secs_op = sec_cnt - secs_done;

        //设置待读入 参数
        select_sector(hd, lba + secs_done, secs_op);
        // 指令设置 -> 读取
        cmd_out(hd->my_channel, CMD_READ_SECTOR);
        // 读写数据时, 阻塞自己 -> 中断处理中会 up
        sem_down(&hd->my_channel->semb);

        if (!busy_wait(hd)) {
            char error[64];
            sprintf(error, "%s read sector %d failed!!!!!!\n", hd->name, lba);
            PANIC(error);
        }
        //转移数据
        read_from_sector(hd, (void*)((u32)buf + secs_done * 512), secs_op);
        secs_done += secs_op;
    }
    // 解锁
    sem_up(&hd->my_channel->sema);
}

// 向硬盘写入数据
void ide_write(disk* hd, u32 lba, void* buf, u32 sec_cnt) {
    sem_down(&hd->my_channel->sema);

    select_disk(hd);

    u32 secs_op;
    u32 secs_done = 0;
    while(secs_done < sec_cnt) {
        if (secs_done + 256 <= sec_cnt) secs_op = 256;
        else secs_op = sec_cnt - secs_done;

        select_sector(hd, lba + secs_done, secs_op);

        cmd_out(hd->my_channel, CMD_WRITE_SECTOR);

        // 开始写数据
        if (!busy_wait(hd)) {
            char error[64];
            sprintf(error, "%s write sector %d failed!!!!!!\n", hd->name, lba);
            PANIC(error);
        }

        write_to_sector(hd, (void*)((u32) buf + secs_done * 512), secs_op);
        // 在硬盘响应期间阻塞自己
        sem_down(&hd->my_channel->semb);
        secs_done += secs_op;
    }
    // 解锁
    sem_up(&hd->my_channel->sema);
}

void hd_handler(u8 irq_no) {
    // 中断处理
    // 一般只 处理 单条通道
    ide_channel *channel = &channels[irq_no - 0x2e];
    if (channel->expecting_intr) {
        channel->expecting_intr = 0;
        sem_up(&channel->semb);
        // 标记此次中断已经被处理, 并且可以开始新的中断
        inb(reg_status(channel));
    }
}

static void swap_pairs_bytes(const char *des, char *res, u32 len) {
    int i;
    for (i = 0; i < len; i += 2) {
        res[i + 1] = *des++;
        res[i] = *des++;
    }
    res[i] = '\0';
}

// 获取硬盘的参数信息
static void identify_disk(disk* hd) {
    char id_info[512];
    select_disk(hd);
    cmd_out(hd->my_channel, CMD_IDENTIFY);
    kprintf("HD:%s\n", hd->name);
//    sem_down(&hd->my_channel->semb);
//
//    if(!busy_wait(hd)){
//        char error[64];
//        sprintf(error, "%s identify failed!!!!!!\n", hd->name);
//        PANIC(error);
//    }
    read_from_sector(hd, id_info, 1);

    // 读取 一个扇区

    char buf[64];
    u8 sn_start = 10 * 2, sn_len = 20, md_start = 27 * 2, md_len = 40;

    swap_pairs_bytes(&id_info[sn_start], buf, sn_len);

    kprintf("   disk %s info:\n      SN: %s\n", hd->name, buf);
    memset(buf, 0, sizeof(buf));
    kprintf(&id_info[md_start], buf, md_len);
    kprintf("      MODULE: %s\n", buf);
    u32 sectors = *(u32 *)&id_info[60 * 2];
    kprintf("      SECTORS: %d\n", sectors);
    kprintf("      CAPACITY: %dMB\n", sectors * 512 / 1024 / 1024);
}


//扫描硬盘特定扇区中的所有分区
static void partition_scan(disk* hd, u32 ext_lba) {
    boot_sector* bs = pmm_malloc(sizeof(boot_sector));
    ide_read(hd, ext_lba, bs, 1);
    partition_table_entry* p = bs->partition_table;
    u8 i = 0;
    //遍历分区表
    while (i++ < 4) {
        //如果是扩展分区
        if (p->fs_type == 0x5){
            if (ext_lba_base != 0) partition_scan(hd, p->start_lba + ext_lba_base);
            else {
                ext_lba_base = p->start_lba;
                partition_scan(hd, p->start_lba);
            }
        }
        else if (p->fs_type != 0) { // 有效分区
            if (ext_lba == 0) {	 // 主分区
                hd->prim_parts[p_no].start_lba = ext_lba + p->start_lba;
                hd->prim_parts[p_no].sec_cnt = p->sec_cnt;
                hd->prim_parts[p_no].my_disk = hd;
                list_add_last(&partition_list, &hd->prim_parts[p_no].part_tag);
                sprintf(hd->prim_parts[p_no].name, "%s%d", hd->name, p_no + 1);
                p_no++;
            } else {
                hd->logic_parts[l_no].start_lba = ext_lba + p->start_lba;
                hd->logic_parts[l_no].sec_cnt = p->sec_cnt;
                hd->logic_parts[l_no].my_disk = hd;
                list_add_last(&partition_list, &hd->logic_parts[l_no].part_tag);
                sprintf(hd->logic_parts[l_no].name, "%s%d", hd->name, l_no + 5);	 // 逻辑分区数字是从5开始,主分区是1～4.
                l_no++;
                if (l_no >= 8)    // 只支持8个逻辑分区,避免数组越界
                    return;
            }
        }
        p++;
    }
    pmm_free(bs);
}


//typedef u8 list_func(list_node *, void *);
// 打印分区信息
static bool partition_info(list_node *node, void *arg) {
    partition* part = node2entry(partition, part_tag, node);
    kprintf("   %s start_lba:0x%x, sec_cnt:0x%x\n",part->name, part->start_lba, part->sec_cnt);
    return 0;
}


void ide_init() {
    kprintf("Start init ide\n");
    // 硬盘信息存储在 物理地址0x475 -> 映射到 偏移0xc0000000上
    u8 hd_num = *((u8 *)(KERNEL_PAGE_OFFSET + 0x475));
    kprintf("Hd num: %d\n", hd_num);
    channel_num = DIV_ROUND_UP(hd_num, 2);
    kprintf("Channel num: %d\n", channel_num);
    ide_channel *channel;
    u8 cnt = 0;
    u8 dev_no = 0;
    // 初始化 处理 每个通道上的硬盘
    while (cnt < channel_num) {
        channel = channels + cnt;
        sprintf(channel->name, "ide%d", cnt);
        // 为每个通道初始化 端口 中断
        switch (cnt) {
            case 0:
                // 1通道 -> 起始端口号
                channel->port_base = 0x1f0;
                channel->irq_no = 0x20 + 14;
                break;
            case 1:
                channel->port_base = 0x170;
                channel->irq_no = 0x20 + 15;
                break;
        }
        channel->expecting_intr = 0;
        // 用来上锁
        sem_init(&channel->sema, 1);
        // 用来阻塞自己, 设为 0
        sem_init(&channel->semb, 0);
        // 读取通道上硬盘
        disk *hd = &channel->devices[dev_no];
        hd->my_channel = channel;
        hd->dev_no = dev_no;
        sprintf(hd->name, "sd%c", (char)('a' + cnt * 2 + dev_no));
        identify_disk(hd);
        partition_scan(hd, 0);
        p_no = 0;
        l_no = 0;
        dev_no++;
        cnt++;
    }
    kprintf("\n all partition: \n");
    list_traversal(&partition_list, partition_info, (void *)NULL);
}

