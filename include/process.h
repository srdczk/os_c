//
// Created by srdczk on 20-3-4.
//
#pragma once

#include "thread.h"

#define DEFAULT_PRIO 31

void print();

void switch_to_user();

void start_process(void *func);

void pde_enable(task_struct *pthread);

void process_enable(task_struct *pthread);

void *create_pde();

void process_exec(void *func, char *name);

