//
// Created by srdczk on 20-2-28.
//

#include "../include/sync.h"


void sem_init(semaphore *sem, u32 val) {
    sem->val = val;
    list_init(&sem->block_list);
}

// semaphore P 操作
void sem_down(semaphore *sem) {
    disable_int();
    while (!sem->val) {
        // 阻塞已经调度的线程 -> 直到up操作完成
        list_add_last(&sem->block_list, &running_thread()->general_tag);
        thread_block(BLOCKED);
    }
    --sem->val;
    enable_int();
}

void sem_up(semaphore *sem) {
    disable_int();
    if (!list_empty(&sem->block_list)) {
        thread_unblock(node2entry(task_struct, general_tag, list_pop_first(&sem->block_list)));
    }
    ++sem->val;
    enable_int();
}



