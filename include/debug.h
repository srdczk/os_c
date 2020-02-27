//
// Created by chengzekun on 20-2-26.
//
#pragma once

void panic(char *filename, int line, const char *func, char *cond);

#define PANIC(...) panic(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NDEBUG
#define ASSERT(CONDITION) ((void) 0)
#else
#define ASSERT(CONDITION) \
if (CONDITION) {} else {  \
    PANIC(#CONDITION);    \
}
#endif
