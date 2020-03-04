//
// Created by srdczk on 20-2-28.
//
#pragma once

#include "thread.h"
#include "isr.h"
#include "list.h"
#include "console.h"

typedef struct {
    u32 val;
    list block_list;
} semaphore;

void sem_init(semaphore *sem, u32 val);

void sem_down(semaphore *sem);

void sem_up(semaphore *sem);

