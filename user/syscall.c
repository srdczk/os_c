//
// Created by srdczk on 20-3-4.
//
#include <stdio.h>
#include "../include/syscall.h"
#include "../include/thread.h"
#include "../include/console.h"
#include "../include/debug.h"
#include "../include/string.h"
#include "../include/fork.h"
#include "../include/fs.h"

static u32 (*syscalls[])(u32 *arg) = {
    sys_getpid,
    sys_write,
    sys_malloc,
    sys_free,
    sys_fork,
    sys_read,
    sys_putc,
    sys_clear,
    sys_getcwd,
    sys_open,
    sys_close,
    sys_lseek,
    sys_unlink,
    sys_mkdir,
    sys_opendir,
    sys_closedir,
    sys_chdir,
    sys_rmdir,
    sys_readdir,
    sys_rewinddir,
    sys_stat
};

u32 sys_getpid(u32 *arg) {
    return running_thread()->pid;
}

u32 getpid() {
    return (u32) _syscall0(SYS_GETPID);
}

u32 sys_write(u32 *arg) {
    char *str = arg[0];
    console_print(str);
    return strlen(str);
}

u32 write(char *str) {
    return (u32) _syscall1(SYS_WRITE, str);
}

u32 sys_malloc(u32 *arg) {
    u32 size = (u32) arg[0];
    return (u32) pmm_malloc(size);
}

void *malloc(u32 size) {
    return (void *) _syscall1(SYS_MALLOC, size);
}

u32 sys_free(u32 *arg) {
    void *ptr = (void *)arg[0];
    pmm_free(ptr);
    return 0;
}

void free(void *ptr) {
    _syscall1(SYS_FREE, ptr);
}

u32 sys_fork(u32 *arg) {
    return ts_fork();
}

u32 fork() {
    return (u32)_syscall0(SYS_FORK);
}

u32 sys_read(u32 *arg) {
    u32 fd = (u32) arg[0];
    void *buf = (void *)arg[1];
    u32 cnt = (u32)arg[2];
    return (u32) fs_read(fd, buf, cnt);
}

int read(int fd, void *buf, u32 cnt) {
    return (int)_syscall3(SYS_READ, fd, buf, cnt);
}

u32 sys_putc(u32 *arg) {
    char c = (char)arg[0];
    console_putc(c, WHITE);
    return 0;
}

void putchar(char c) {
    _syscall1(SYS_PUC, c);
}

u32 sys_clear(u32 *arg) {
    console_clear();
    return 0;
}

void clear() {
    _syscall0(SYS_CLR);
}

u32 sys_getcwd(u32 *arg) {
    char *buf = (char *)arg[0];
    u32 size = (u32)arg[1];
    return (u32) fs_getcwd(buf, size);
}

char *getcwd(char *buf, u32 size) {
    return (char *) _syscall2(SYS_GETCWD, buf, size);
}

u32 sys_open(u32 *arg) {
    char *name = (char *)arg[0];
    u8 flag = (u8)arg[1];
    return (u32) fs_open(name, flag);
}

int open(char *pathname, u8 flag) {
    return (int) _syscall2(SYS_OPEN, pathname, flag);
}

u32 sys_close(u32 *arg) {
    u32 fd = (u32) arg[0];
    return (u32) fs_close(fd);
}

int close(int fd) {
    return (int) _syscall1(SYS_CLOSE, fd);
}

u32 sys_lseek(u32 *arg) {
    u32 fd = (u32) arg[0];
    int offset = (int) arg[1];
    u8 seek = (u8) arg[2];
    return (u32) fs_lseek(fd, offset, seek);
}

int lseek(int fd, int offset, u8 flag) {
    return (int) _syscall3(SYS_LSEEK, fd, offset, flag);
}

u32 sys_unlink(u32 *arg) {
    const char *name = (const char *)arg[0];
    return (u32) fs_unlink(name);
}

int unlink(const char *pathname) {
    return (int) _syscall1(SYS_UNLINK, pathname);
}

u32 sys_mkdir(u32 *arg) {
    const char *name = (const char *) arg[0];
    return (u32) fs_mkdir(name);
}

int mkdir(const char *pathname) {
    return (int)_syscall1(SYS_MKDIR, pathname);
}

u32 sys_opendir(u32 *arg) {
    const char *name = (const char *)arg[0];
    return (u32) fs_opendir(name);
}

dir *opendir(const char *name) {
    return (dir *)_syscall1(SYS_OPENDIR, name);
}

u32 sys_closedir(u32 *arg) {
    dir *d = (dir *)arg[0];
    return (u32) fs_closedir(d);
}

void closedir(dir *d) {
    _syscall1(SYS_CLOSEDIR, d);
}

u32 sys_chdir(u32 *arg) {
    const char *path = (const char *)arg[0];
    return (u32) fs_chdir(path);
}

int chdir(const char *path) {
    return (int) _syscall1(SYS_CHDIR, path);
}

u32 sys_rmdir(u32 *arg) {
    const char *path = (const char *)arg[0];
    return (u32)fs_rmdir(path);
}

int rmdir(const char *pathname) {
    return (int) _syscall1(SYS_RMDIR, pathname);
}

u32 sys_readdir(u32 *arg) {
    dir *d = (dir *)arg[0];
    return (u32) fs_readdir(d);
}

dir_entry *readdir(dir *d) {
    return (dir_entry *) _syscall1(SYS_READDIR, d);
}

u32 sys_rewinddir(u32 *arg) {
    dir *d = (dir *)arg[0];
    fs_rewinddir(d);
    return 0;
}

void rewinddir(dir *d) {
    _syscall1(SYS_REWINDDIR, d);
}

u32 sys_stat(u32 *arg) {
    const char *path = (const char *)arg[0];
    stat *buf = (stat *)arg[1];
    return (u32)fs_stat(path, buf);
}

int file_stat(const char *path, stat *s) {
    return (int) _syscall2(SYS_STAT, path, s);
}

void syscall(int_frame *tf) {
    // 支持三个 eax, ebx, ecx, edx 传参
    u32 arg[4];
    int num = tf->eax;
    if (num >= 0 && num < SYSCALL_SIZE) {
        if (syscalls[num] != NULL) {
            arg[0] = tf->ebx;
            arg[1] = tf->ecx;
            arg[2] = tf->edx;
            tf->eax = syscalls[num](arg);
            return ;
        }
    }
    PANIC("WRONG SYSCALL NUMBER!");
}

