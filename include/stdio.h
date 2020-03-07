//
// Created by srdczk on 20-3-5.
//
#pragma once

#include "types.h"

typedef char *va_list;

u32 vsprintf(char *str, const char *format, va_list ap);

u32 printf(const char *format, ...);

u32 kprintf(const char *format, ...);

u32 sprintf(char *buff, const char *format, ...);

