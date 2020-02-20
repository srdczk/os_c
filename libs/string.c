#include "../include/string.h"


void memcpy(char *src, char *des, int len) {
    int i;
    for (i = 0; i < len; ++i) {
        *(des + i) = *(src + i);
    }
}

void memset(char *src, char val, int len) {
    int i;
    for (i = 0; i < len; ++i) {
        *(src + i) = val;
    }
}
