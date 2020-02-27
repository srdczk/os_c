#pragma once

#include "types.h"

void memcpy(const char *src, char *des, int len);

void memset(char *src, char val, int len);

int memcmp(const char *a, const char *b, int len);

char *strcpy(const char *src, char *des);

u32 strlen(const char *src);

u8 strcmp(const char *a, const char *b);

char *strchr(const char *src, char val);

char *strrchr(const char *src, char val);

char *strcat(const char *src, char *des);

u32 strchrs(const char *src, char val);

void to_hex_string(u32 i, char s[]);

void to_dec_string(u32 i, char s[]);
