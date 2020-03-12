//
// Created by srdczk on 20-3-12.
//
#include "../include/fork.h"
#include "../include/thread.h"
#include "../include/string.h"
#include "../include/arena.h"
#include "../include/process.h"
#include "../include/idt.h"
#include "../include/file.h"

extern void int_exit();

static int copy_pcb_stack0(task_struct *child, task_struct *parent) {
    // 拷贝所有内核栈空间
    memcpy(parent, child, PAGE_SIZE);
    child->pid = global_pid++;
    child->running_ticks = 0;
    child->ticks = child->priority;
    child->parent_pid = parent->pid;
    child->general_tag.pre = child->general_tag.next = NULL;
    child->global_tag.pre = child->global_tag.next = NULL;
    area_desc_init(child->user_arena);
    // page count
    // 父 进程, 虚拟地址位图的复制
    child->user_pool.addr_start = USER_OFFSET;
    u32 page_cnt = DIV_ROUND_UP((KERNEL_PAGE_OFFSET - USER_OFFSET) / (PAGE_SIZE * 8), PAGE_SIZE);
    child->user_pool.bmap.map = kmalloc_page(page_cnt, (u32 *)parent->pgdir);
    child->user_pool.bmap.map_len = (KERNEL_PAGE_OFFSET - USER_OFFSET) / (PAGE_SIZE * 8);
    child->pgdir = (u32) create_pde();
    memcpy(parent->user_pool.bmap.map, child->user_pool.bmap.map, page_cnt * PAGE_SIZE);
    strcat("_fork", child->name);
    return 0;
}

static void copy_pcb_stack3(task_struct *child, task_struct *parent, void *kernel_buf) {
    // 申请内核缓冲区, 复制父进程用户空间的内容
    u32 index = 0;
    u32 va;
    while (index < 8 * parent->user_pool.bmap.map_len) {
        u32 x = index / 8, y = index % 8;
        if ((parent->user_pool.bmap.map[x] & (1 << y))) {
            va = parent->user_pool.addr_start + index * PAGE_SIZE;
            memcpy(va, kernel_buf, PAGE_SIZE);
            // 使用 child 页表, 并且申请相应的虚拟内存, 不改变bitmap
            pde_enable(child);
            get_a_page((u32 *)child->pgdir, va);
            memcpy(kernel_buf, va, PAGE_SIZE);
            // 恢复父进程页表
            pde_enable(parent);
        }
        index++;
    }
}

static void make_child(task_struct *child) {
    int_frame *frame = ((u32)child + PAGE_SIZE - sizeof(int_frame));
    // 子进程的返回值为 0
    frame->eax = 0;
    u32 *ret_addr = (u32 *)frame - 1;
    u32 *esi_ptr = (u32 *)frame - 2;
    u32 *edi_ptr = (u32 *)frame - 3;
    u32 *ebx_ptr = (u32 *)frame - 4;
    u32 *ebp_ptr = (u32 *)frame - 5;
    *ret_addr = (u32)int_exit;
    *esi_ptr = *edi_ptr = *ebx_ptr = *ebp_ptr = 0;
    child->self_stack = (u32)ebp_ptr;
}

static void update_inode(task_struct *pthread) {
    int local_fd = 3, global_fd = -1;
    while (local_fd < MAX_OPEN_FILE) {
        global_fd = pthread->fd_table[local_fd++];
        if (global_fd != -1) {
            file_table[global_fd].fd_inode->i_open_cnts++;
        }
    }
}

static int copy_process(task_struct *child, task_struct *parent) {
    // 申请一页的内核缓冲区
    void *kernel_buf = kmalloc_page(1, (u32 *)parent->pgdir);
    copy_pcb_stack0(child, parent);
    copy_pcb_stack3(child, parent, kernel_buf);
    make_child(child);
    update_inode(child);
    // 重新回收
    return 0;
}

u32 ts_fork() {
    task_struct *parent = running_thread();
    task_struct *child = kmalloc_page(1, (u32 *)parent->pgdir);
    copy_process(child, parent);
    list_add_last(&ready_list, &child->general_tag);
    list_add_last(&global_list, &child->global_tag);
    return child->pid;
}

