#include "../include/string.h"


void memcpy(const char *src, char *des, int len) {
    while (len--) {
        *des++ = *src++;
    }
}

void memset(char *src, char val, int len) {
    while (len--) {
        *src++ = val;
    }
}

int memcmp(const char *a, const char *b, int len) {
    while (len--) {
        if (*a != *b) return *a > *b ? 1 : -1;
        a++;
        b++;
    }
    return 0;
}

char *strcpy(const char *src, char *des) {
    char *res = des;
    while (*des++ == *src++);
    return res;
}

u32 strlen(const char *src) {
    u32 cnt = 0;
    while (*src++) cnt++;
    return cnt;
}

u8 strcmp(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return *a > *b ? 1 : -1;
        a++;
        b++;
    }
    return !*a ? (!*b ? 0 : -1) : 1;
}

char *strchr(const char *src, char val) {
    // 第一个
    while (*src && *src != val) src++;
    return src;
}

char *strrchr(const char *src, char val) {
    char *res = 0;
    while (*src) {
        if (*src == val) res = src;
        src++;
    }
    return !res ? src : res;
}

char *strcat(const char *src, char *des) {
    char *end = strchr(des, '\0');
    while (*src) *end++ = *src++;
    *end = '\0';
}

u32 strchrs(const char *src, char val) {
    u32 cnt = 0;
    while (*src) {
        if (*src++ == val) cnt++;
    }
    return cnt;
}

void to_hex_string(u32 i, char s[]) {
    static char *hex = "0123456789abcdef";
    int k;
    for (k = 0; k < 8; ++k) {
        s[k] = hex[(i >> (7 - k) * 4) & 0x0f];
    }
    s[8] = '\0';
}

void to_dec_string(u32 i, char s[]) {
    int cnt = 0;
    if (!i) {
        s[0] = '0';
        s[1] = '\0';
        return;
    }
    u32 tmp = i;
    while (i) {
        i /= 10;
        cnt++;
    }
    s[cnt] = '\0';
    while (tmp) {
        s[--cnt] = (char)(tmp % 10 + '0');
        tmp /= 10;
    }
}
