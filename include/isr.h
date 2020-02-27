#pragma once

// 中断服务例程
//
#include "idt.h"

// 中断 是否开启
typedef enum {
    INT_OFF,
    INT_ON
} int_status;

int_status get_int_status();

int_status set_int_status(int_status status);

int_status enable_int();

int_status disable_int();

void idt_init();
const char *int_name(int int_no);
void int_dispatch(int_frame *tf);

