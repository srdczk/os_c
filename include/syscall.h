//
// Created by srdczk on 20-3-4.
//
#pragma once

#include "types.h"
#include "idt.h"
#include "pmm.h"
#include "dir.h"

#define SYS_GETPID 0
#define SYS_WRITE 1
#define SYS_MALLOC 2
#define SYS_FREE 3
#define SYS_FORK 4
#define SYS_READ 5
#define SYS_PUC 6
#define SYS_CLR 7
#define SYS_GETCWD 8
#define SYS_OPEN 9
#define SYS_CLOSE 10
#define SYS_LSEEK 11
#define SYS_UNLINK 12
#define SYS_MKDIR 13
#define SYS_OPENDIR 14
#define SYS_CLOSEDIR 15
#define SYS_CHDIR 16
#define SYS_RMDIR 17
#define SYS_READDIR 18
#define SYS_REWINDDIR 19
#define SYS_STAT 20
// 最多 32 个 系统调用
#define SYSCALL_SIZE 0x20

// 无参数的系统调用
#define _syscall0(NUM) ({ \
    int retval;           \
    asm volatile (        \
            "int $0x80"   \
            : "=a"(retval)\
            : "a"(NUM)    \
            : "memory"    \
            );            \
    retval;               \
})

// 一个参数的系统调用
 #define _syscall1(NUM, ARG1) ({ \
     int retval;           \
     asm volatile (        \
             "int $0x80"   \
             : "=a"(retval)\
             : "a"(NUM), "b"(ARG1)    \
             : "memory"    \
             );            \
     retval;               \
 })

/* 两个参数的系统调用 */
#define _syscall2(NUMBER, ARG1, ARG2) ({		       \
   int retval;						       \
   asm volatile (					       \
   "int $0x80"						       \
   : "=a" (retval)					       \
   : "a" (NUMBER), "b" (ARG1), "c" (ARG2)		       \
   : "memory"						       \
   );							       \
   retval;						       \
})

/* 三个参数的系统调用 */
#define _syscall3(NUMBER, ARG1, ARG2, ARG3) ({		       \
   int retval;						       \
   asm volatile (					       \
      "int $0x80"					       \
      : "=a" (retval)					       \
      : "a" (NUMBER), "b" (ARG1), "c" (ARG2), "d" (ARG3)       \
      : "memory"					       \
   );							       \
   retval;						       \
})


u32 sys_getpid(u32 *arg);

u32 getpid();

u32 sys_write(u32 *arg);

u32 write(char *str);

u32 sys_malloc(u32 *arg);

void *malloc(u32 size);

u32 sys_free(u32 *arg);

void free(void *ptr);

u32 sys_fork(u32 *arg);

u32 fork();

u32 sys_read(u32 *arg);

int read(int fd, void *buf, u32 cnt);

u32 sys_putc(u32 *arg);

void putchar(char c);

u32 sys_clear(u32 *arg);

void clear();

u32 sys_getcwd(u32 *arg);

char *getcwd(char *buf, u32 size);

u32 sys_open(u32 *arg);

int open(char *pathname, u8 flag);

u32 sys_close(u32 *arg);

int close(int fd);

u32 sys_lseek(u32 *arg);

int lseek(int fd, int offset, u8 flag);

u32 sys_unlink(u32 *arg);

int unlink(const char *pathname);

u32 sys_mkdir(u32 *arg);

int mkdir(const char *pathname);

u32 sys_opendir(u32 *arg);

dir *opendir(const char *name);

u32 sys_closedir(u32 *arg);

void closedir(dir *d);

u32 sys_chdir(u32 *arg);

int chdir(const char *path);

u32 sys_rmdir(u32 *arg);

int rmdir(const char *pathname);

u32 sys_readdir(u32 *arg);

dir_entry *readdir(dir *d);

u32 sys_rewinddir(u32 *arg);

void rewinddir(dir *d);

u32 sys_stat(u32 *arg);

int file_stat(const char *path, stat *s);

void syscall(int_frame *tf);

