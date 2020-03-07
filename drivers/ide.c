#include "../include/ide.h"
#include "../include/x86.h"
#include "../include/pmm.h"

#define IRQ_IDE1 0x2e
#define IRQ_IDE2 0x2f

#define SECTSIZE 512

#define ISA_DATA                0x00
#define ISA_ERROR               0x01
#define ISA_PRECOMP             0x01
#define ISA_CTRL                0x02
#define ISA_SECCNT              0x02
#define ISA_SECTOR              0x03
#define ISA_CYL_LO              0x04
#define ISA_CYL_HI              0x05
#define ISA_SDH                 0x06
#define ISA_COMMAND             0x07
#define ISA_STATUS              0x07

#define IDE_BSY                 0x80
#define IDE_DRDY                0x40
#define IDE_DF                  0x20
#define IDE_DRQ                 0x08
#define IDE_ERR                 0x01

#define IDE_CMD_READ            0x20
#define IDE_CMD_WRITE           0x30
#define IDE_CMD_IDENTIFY        0xEC

#define IDE_IDENT_SECTORS       20
#define IDE_IDENT_MODEL         54
#define IDE_IDENT_CAPABILITIES  98
#define IDE_IDENT_CMDSETS       164
#define IDE_IDENT_MAX_LBA       120
#define IDE_IDENT_MAX_LBA_EXT   200

#define IO_BASE0                0x1F0
#define IO_BASE1                0x170
#define IO_CTRL0                0x3F4
#define IO_CTRL1                0x374

#define MAX_IDE                 4
#define MAX_NSECS               128
#define MAX_DISK_NSECS          0x10000000U
#define VALID_IDE(ideno)        (((ideno) >= 0) && ((ideno) < MAX_IDE) && (ide_devices[ideno].valid))


channel channels[2] = {
        {IO_BASE0, IO_CTRL0},
        {IO_BASE1, IO_CTRL1}
};

#define IO_BASE(ideno)          (channels[(ideno) >> 1].base)
#define IO_CTRL(ideno)          (channels[(ideno) >> 1].ctrl)

// 监听两个通道, 每个通道最多 ->
ide_device ide_devices[MAX_IDE];

u32 ext_lba_base = 0;

// 主分区和逻辑分区编号
u8 p_no = 0, l_no = 0;

// 分区链表
list part_list;

// 分区表项目
typedef struct {
        // 是否 引导
        u8  bootable;
        //起始磁头号,扇区,柱面
        u8  start_head;
        u8  start_sec;
        u8  start_chs;
        //分区类型
        u8  fs_type;
        //结束磁头号,扇区,柱面
        u8  end_head;
        u8  end_sec;
        u8  end_chs;
        //起始扇区lba
        u32 start_lba;
        //扇区数目
        u32 sec_cnt;
} __attribute__ ((packed)) partition_entry;

// 引导扇区
typedef struct {
    // 代码
    u8 other[446];
    // 分区表项
    partition_entry partition_table[4];
    // 结束符 0x55AA, (小端法 16 进制直接0xAA55)
    u16 signature;
} __attribute__ ((packed)) boot_sector;

static int ide_wait_ready(u16 iobase, bool check_error) {
    int r;
    // 阻塞
    while ((r = inb(iobase + ISA_STATUS)) & IDE_BSY);
    if (check_error && (r & (IDE_DF | IDE_ERR)) != 0) {
        return -1;
    }
    return 0;
}

// 扫描硬盘分区
static void partition_scan(u16 devno, u32 ext_lba) {
    char name[] = "sda";
    boot_sector *bs = pmm_malloc(sizeof(boot_sector));
    kprintf("bs:%x\n", (u32)bs);
    // 读入引导扇区
    int res = ide_read_secs(devno, ext_lba, bs, 1);
    kprintf("res:%d\n", res);
    kprintf("YES\n");
    partition_entry *p = bs->partition_table;
    kprintf("start:lba0x%x\n", p->start_lba);
    kprintf("sig:%x\n", bs->signature);
    kprintf("p->type:%d\n", p->fs_type);
    u8 i = 0;
    // 深度优先搜索所有分区
    while (i++ < 4) {
        // 拓展分区
        if (p->fs_type == 0x5) {
            // 如果 已经
            if (ext_lba_base != 0) {
                kprintf("X1\n");
                partition_scan(devno, p->start_lba + ext_lba_base);
            } else {
                kprintf("X2\n");
                ext_lba_base = p->start_lba;
                partition_scan(devno, p->start_lba);
            }
            // 有效分区
        } else if (p->fs_type != 0) {
            if(ext_lba == 0) {
                // 主分区
                kprintf("X3\n");
                ide_devices[devno].prim_parts[p_no].start_lba = ext_lba + p->start_lba;
                ide_devices[devno].prim_parts[p_no].sec_num = p->sec_cnt;
                ide_devices[devno].prim_parts[p_no].devno = devno;
                kprintf("NIMA\n");
                list_add_last(&part_list, &ide_devices[devno].prim_parts[p_no].part_tag);
                sprintf(ide_devices[devno].prim_parts[p_no].name, "%s%d", name, p_no + 1);
                kprintf("WULIU\n");
                p_no++;
            } else {
                kprintf("X4\n");
                // 逻辑分区
                ide_devices[devno].logic_parts[l_no].start_lba =
                ide_devices[devno].logic_parts[l_no].start_lba = ext_lba + p->start_lba;
                ide_devices[devno].logic_parts[l_no].sec_num = p->sec_cnt;
                ide_devices[devno].logic_parts[l_no].devno = devno;
                list_add_last(&part_list, &ide_devices[devno].logic_parts[l_no].part_tag);
                sprintf(ide_devices[devno].logic_parts[l_no].name, "%s%d", name, l_no + 5);
                l_no++;
                if (l_no >= 8) return;
            }
        }
        p++;
    }
    pmm_free(bs);
}

static bool partition_info(list_node *node, void *arg) {
    partition *part = node2entry(partition, part_tag, node);
    kprintf("    %s start_lba:0x%x, sec_num:0x%x\n", part->name, part->start_lba, part->sec_num);
    return 0;
}

void ide_init() {
    u16 ideno, iobase;
    for (ideno = 0; ideno < MAX_IDE; ideno ++) {
        ide_devices[ideno].valid = 0;

        iobase = IO_BASE(ideno);

        ide_wait_ready(iobase, 0);

        outb(iobase + ISA_SDH, 0xe0 | ((ideno & 1) << 4));
        ide_wait_ready(iobase, 0);

        outb(iobase + ISA_COMMAND, IDE_CMD_IDENTIFY);
        ide_wait_ready(iobase, 0);

        if (inb(iobase + ISA_STATUS) == 0 || ide_wait_ready(iobase, 1) != 0) {
            continue ;
        }

        ide_devices[ideno].valid = 1;

        u32 buffer[128];
        insw(iobase + ISA_DATA, buffer, sizeof(buffer) / sizeof(u32));

        u8 *ident = (u8 *)buffer;
        u32 sectors;
        u32 cmdsets = *(u32 *)(ident + IDE_IDENT_CMDSETS);
        if (cmdsets & (1 << 26)) sectors = *(u32 *)(ident + IDE_IDENT_MAX_LBA_EXT);
        else sectors = *(u32 *)(ident + IDE_IDENT_MAX_LBA);
        ide_devices[ideno].sets = cmdsets;
        ide_devices[ideno].size = sectors;

        u8 *model = ide_devices[ideno].model, *data = ident + IDE_IDENT_MODEL;
        u32 i, length = 40;
        for (i = 0; i < length; i += 2) {
            model[i] = data[i + 1], model[i + 1] = data[i];
        }
        do {
            model[i] = '\0';
        } while (i -- > 0 && model[i] == ' ');
        if (!ide_devices[ideno].size) break;
        kprintf("ide %d %d(sectors) '%s'\n", ideno, ide_devices[ideno].size, ide_devices[ideno].model);
    }
    irq_enable(IRQ_IDE1);
    irq_enable(IRQ_IDE2);
    // 初始化 链表
    list_init(&part_list);
    // 处理 0 号硬盘, 检查分区 情况
    partition_scan(0, 0);
    list_traversal(&part_list, partition_info, NULL);
    kprintf("ide_init done\n");
}

bool ide_device_valid(u16 ideno) {
    return VALID_IDE(ideno);
}

u32 ide_device_size(u16 ideno) {
    if (ide_device_valid(ideno)) {
        return ide_devices[ideno].size;
    }
    return 0;
}

int ide_read_secs(u16 ideno, u32 secno, void *dst, u32 nsecs) {
    u16 iobase = IO_BASE(ideno), ioctrl = IO_CTRL(ideno);

    ide_wait_ready(iobase, 0);

    outb(ioctrl + ISA_CTRL, 0);
    outb(iobase + ISA_SECCNT, nsecs);
    outb(iobase + ISA_SECTOR, secno & 0xFF);
    outb(iobase + ISA_CYL_LO, (secno >> 8) & 0xFF);
    outb(iobase + ISA_CYL_HI, (secno >> 16) & 0xFF);
    outb(iobase + ISA_SDH, 0xE0 | ((ideno & 1) << 4) | ((secno >> 24) & 0xF));
    outb(iobase + ISA_COMMAND, IDE_CMD_READ);

    int ret = 0;
    for (; nsecs > 0; nsecs --, dst += SECTSIZE) {
        if ((ret = ide_wait_ready(iobase, 1)) != 0) {
            goto out;
        }
        insl(iobase, dst, SECTSIZE / sizeof(u32));
    }

    out:
    return ret;
}

int ide_write_secs(u16 ideno, u32 secno, void *src, u32 nsecs) {
    u16 iobase = IO_BASE(ideno), ioctrl = IO_CTRL(ideno);

    ide_wait_ready(iobase, 0);

    outb(ioctrl + ISA_CTRL, 0);
    outb(iobase + ISA_SECCNT, nsecs);
    outb(iobase + ISA_SECTOR, secno & 0xFF);
    outb(iobase + ISA_CYL_LO, (secno >> 8) & 0xFF);
    outb(iobase + ISA_CYL_HI, (secno >> 16) & 0xFF);
    outb(iobase + ISA_SDH, 0xE0 | ((ideno & 1) << 4) | ((secno >> 24) & 0xF));
    outb(iobase + ISA_COMMAND, IDE_CMD_WRITE);

    int ret = 0;
    for (; nsecs > 0; nsecs --, src += SECTSIZE) {
        if ((ret = ide_wait_ready(iobase, 1)) != 0) {
            goto out;
        }
        outsl(iobase, src, SECTSIZE / sizeof(u32));
    }
    out:
    return ret;
}