#pragma once

#include "thread.h"
//可调进程链表
extern task_struct *running_head;
// 正在运行的
extern task_struct *cur;

extern char kernel_stack[];
// 初始化 任务调度
void init_schedule();
// 调度
void schedule();
// 任务切换
void change_task_to(task_struct *next);

