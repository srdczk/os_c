#pragma once


#define NULL 0

#define EFLAGS_MBS (1 << 1)
#define EFLAGS_IF_1 (1 << 9)
#define EFLAGS_IF_0 (0)
#define EFLAGS_IOPL_3 (3 << 12)
#define EFLAGS_IOPL_0 (0)

// 向上取整
#define DIV_ROUND_UP(X, Y) ((X + Y - 1) / (Y))

// 文件信息的定义
// 分区最大文件
#define MAX_FILES_PER_PART 4096
// 扇区 bit
#define BITS_PER_SECTOR 4096
#define SECTOR_SIZE 512
#define BLOCK_SIZE SECTOR_SIZE

typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;
typedef unsigned char u8;
typedef char s8;
typedef u8 bool;

// 文件类型枚举
typedef enum {
    UNKNOWN,
    REGULAR,
    DIRECTORY
}  file_type;

// 打开文件的选项
typedef enum {
    O_RDONLY,
    O_WRONLY,
    O_RDWR,
    O_CREAT = 4 // 0x0 0x1 0x2 0x4
} file_flag;

typedef enum {
    SEEK_SET = 1,
    SEEK_CUR,
    SEEK_END
} file_seek;

// 文件属性
typedef struct {
    u32 st_ino; //inode
    u32 st_size; // 大小
    file_type st_type; // 文件类型
} stat;

typedef struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) descriptor;

