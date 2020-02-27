#include "../include/schedule.h"

task_struct *running_head = 0;

task_struct *cur = 0;

extern switch_to(context *, context *);

void init_schedule() {
    cur = (task_struct *)((u32)kernel_stack);
    cur->state = RUNNABLE;
    cur->pid = global_pid++;
    cur->stack = cur;
    cur->next = cur;
    running_head = cur;
}

void schedule() {
    if (cur) change_task_to(cur->next);
}

void change_task_to(task_struct *next) {
    if (cur != next) {
        task_struct *pre = cur;
        cur = next;
        switch_to(&(pre->text), &(cur->text));
    }
}
