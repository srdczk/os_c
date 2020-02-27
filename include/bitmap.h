#pragma once

#include "types.h"

#define BIT_MASK 0x01

typedef struct {
    u32 map_len;
    char *map;
} bitmap;

void bitmap_init(bitmap *bmap);

char bitmap_get(bitmap *bmap, u32 index);

void bitmap_set(bitmap *bmap, u32 index, char val);

int bitmap_apply(bitmap *bmap, u32 cnt);
