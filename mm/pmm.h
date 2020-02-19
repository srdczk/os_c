#pragma once

#include "../libs/mem.h"
#include "../libs/string.h"

typedef struct {
    // 页表每一项的结构
    unsigned present    : 1;
    unsigned rw         : 1;
    unsigned user       : 1;
    unsigned accessed   : 1;
    unsigned dirty      : 1;
    unsigned unused     : 7;
    unsigned frame      : 20;
} page_entry;


typedef struct {
    page_entry pages[1024];
} page_table_entry;

typedef struct {
    page_table_entry *tables[1024];
    u32 tablePhysicals[1024];
    u32 physicalAddr;
} page_directory_entry;


void page_init();

void enable_directory(page_directory_entry *entry);

page_entry *get_page(u32 addr, int make, page_directory_entry *entry);
