#pragma once

// 中断服务例程
//
#include "idt.h"
void idt_init();
const char *int_name(int int_no);
void int_dispatch(int_frame *tf);

