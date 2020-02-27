#include "../include/bitmap.h"
#include "../include/string.h"
#include "../include/debug.h"

#define BITMAP_INDEX(x) ((x) / 8)
#define BITMAP_OFFSET(x) ((x) % 8)

void bitmap_init(bitmap *bmap) {
    memset(bmap->map, '\0', bmap->map_len);
}

char bitmap_get(bitmap *bmap, u32 index) {
    return (bmap->map[BITMAP_INDEX(index)] & (BIT_MASK << BITMAP_OFFSET(index))) ? 1 : 0;
}

void bitmap_set(bitmap *bmap, u32 index, char val) {
    ASSERT(val == 0 || val == 1);
    u32 bindex = BITMAP_INDEX(index);
    u32 boffset = BITMAP_OFFSET(index);
    if (val) bmap->map[bindex] |= BIT_MASK << (boffset);
    else bmap->map[bindex] &= ~(BIT_MASK << boffset);
}

int bitmap_apply(bitmap *bmap, u32 cnt) {
    u32 index = 0;
    u32 bmax = (u32) (bmap->map_len * 8);
    while (index < bmap->map_len && !(0xff ^ bmap->map[index])) index++;
    if (index == bmap->map_len) return -1;
    index *= 8;
    while (index < bmax) {
        u32 count = 0;
        while (index < bmax && count < cnt) {
            if (!bitmap_get(bmap, index++)) count++;
            else break;
        }
        if (count == cnt) return index - count;
    }
    return -1;
}


