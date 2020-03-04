//
// Created by srdczk on 20-3-4.
//
#pragma once

#include "thread.h"

#define DEFAULT_PRIO 31
// 用户进程申请内存的起始位置
#define USER_OFFSET 0x00804800
// 计算存储位图需要多少页
#define DIV_ROUND_UP(X, Y) ((X + Y - 1) / (Y))

void print();

void switch_to_user();

void start_process(void *func);

void pde_enable(task_struct *pthread);

void user_pool_init(task_struct *thread);

void process_enable(task_struct *pthread);

void *create_pde();

void process_exec(void *func, char *name);

