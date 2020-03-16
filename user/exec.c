//
// Created by srdczk on 20-3-16.
//
#include "../include/exec.h"
#include "../include/pmm.h"
#include "../include/thread.h"
#include "../include/fs.h"
#include "../include/string.h"
#include "../include/idt.h"

// 将文件描述符 指向的文件中, 偏移为 offset, 大小 size, 加载到虚拟内存 va 位置
static bool segment_load(u32 fd, u32 offset, u32 size, u32 va) {
    // 第一个分配的页框
    u32 first_page = va & 0xfffff000;
    u32 left_in_page = PAGE_SIZE - (va & 0x00000fff);
    u32 pages;
    if (size > left_in_page) {
        u32 left_size = size - left_in_page;
        pages = DIV_ROUND_UP(left_size, PAGE_SIZE) + 1;
    } else {
        pages = 1;
    }
    u32 cnt = 0;
    u32 addr = first_page;
    u32 *pde = (u32 *)running_thread()->pgdir;
    // 为进程分配内存, 查看是否存在
    while (cnt < pages) {
        u32 pde_index = PDE_INDEX(addr);
        u32 pte_index = PTE_INDEX(addr);
        if (!pde[pde_index]) pde[pde_index] = alloc_page() | PG_USER | PG_PRESENT | PG_RW;
        u32 *pte = (u32 *)(PTE_OFFSET + (pde_index) * PAGE_SIZE);
        if (!pte_index) pte[pte_index] = alloc_page() | PG_USER | PG_PRESENT | PG_RW;
        addr += PAGE_SIZE;
        cnt++;
    }
    fs_lseek(fd, offset, SEEK_SET);
    fs_read(fd, (void *)addr, size);
    return 1;
}

static int load(const char *pathname) {
    // 指定的路径加载 elf 文件头
    int res = -1;
    elfhdr elf_header;
    proghdr prog_header;
    int fd = fs_open(pathname, O_RDONLY);
    if (fd == -1) return -1;
    // 已经打开了文件
    if (fs_read(fd, &elf_header, sizeof(elf_header)) != sizeof(elf_header)) {
        res = -1;
        goto out;
    }
    // 检验 elf 头
    if (memcmp("\177ELF\1\1\1", elf_header.e_elf, 7) || elf_header.e_type != 2 \
     || elf_header.e_machine != 3 || elf_header.e_version != 1 || elf_header.e_phnum > 1024 \
     || elf_header.e_phentsize != sizeof(elf_header)) {
        res = -1;
        goto out;
    }
    u32 phead_offset = elf_header.e_phoff;
    u32 phead_size = elf_header.e_phentsize;
    u32 index = 0;
    while (index < elf_header.e_phnum) {
        memset(&prog_header, '\0', phead_size);
        fs_lseek(fd, phead_offset, SEEK_SET);
        if (fs_read(fd, &prog_header, phead_size) != phead_size) {
            res = -1;
            goto out;
        }
        if (PT_LOAD == prog_header.p_type) {
            if (!segment_load(fd, prog_header.p_offset, prog_header.p_filesz, prog_header.p_va)) {
                res = -1;
                goto out;
            }
        }
        phead_offset += elf_header.e_phentsize;
        index++;
    }
    out:
    fs_close(fd);
    return res;
}

int kernel_exec(const char *name, const char **av) {
    u32 ac = 0;
    while (av[ac]) ac++;
    int point = load(name);
    if (point == -1) return -1;
    task_struct *cur = running_thread();
    u32 len = strlen(name);
    memcpy(name, cur->name, len);
    cur->name[len] = '\0';
    int_frame *frame = (int_frame *) ((u32) cur + PAGE_SIZE - sizeof(int_frame));
    frame->ebx = (u32) av;
    frame->ecx = ac;
    frame->eip = (u32)point;
    frame->useresp = 0xc0000000;
    // 直接中断退出
    asm volatile ("movl $0, %%esp; jmp int_exit" :: "g"((u32) frame) : "memory");
    return 0;
}

