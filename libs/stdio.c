//
// Created by srdczk on 20-3-5.
//

#include "../include/syscall.h"
#include "../include/stdio.h"
#include "../include/string.h"
#include "../include/console.h"

#define va_start(ap, x) ap = (va_list)&x
#define va_arg(ap, x) *((x *)(ap += 4))
#define va_end(ap) ap = NULL

static void swap(char *a, char *b) {
    char tmp;
    while (a < b) {
        tmp = *a;
        *a++ = *b;
        *b-- = tmp;
    }
}

// 转变成 base 进制字符串
static u32 itoa(u32 val, char *buff, u8 base) {
    if (!val) {
        *buff++ = '0';
        return 1;
    }
    u8 mod;
    char *start = buff;
    u32 cnt = 0;
    while (val) {
        if ((mod = val % base) < 10) *buff++ = (char)(mod + '0');
        else *buff++ = (char)(mod - 10 + 'a');
        val /= base;
        cnt++;
    }
    swap(start, buff - 1);
    return cnt;
}

u32 vsprintf(char *str, const char *format, va_list ap) {
    // 参数列表 ap
    char *s_ptr = str;
    const char *f_ptr = format;
    char tmp;
    char *arg_s;
    u32 arg;
    int arg_d;
    while (*f_ptr) {
        if (*f_ptr != '%') {
            *s_ptr++ = *f_ptr++;
            continue;
        }
        tmp = *(++f_ptr);
        switch (tmp) {
            // 16 进制输出
            case 'x':
                arg =  va_arg(ap, u32);
                s_ptr += itoa(arg, s_ptr, 16);
                break;
                // 字符串
            case 's':
                arg_s = va_arg(ap, char *);
                strcpy(arg_s, s_ptr);
                s_ptr += strlen(arg_s);
                break;
                // 十进制输出
            case 'd':
                arg_d = va_arg(ap, int);
                if (arg_d < 0) {
                    *s_ptr++ = '-';
                    arg_d = -arg_d;
                }
                s_ptr += itoa((u32) arg_d, s_ptr, 10);
                break;
                // 输出字符
            case 'c':
                *s_ptr++ = va_arg(ap, char);
                break;
        }
        f_ptr++;
    }
    *s_ptr = '\0';
    return strlen(str);
}

u32 printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buff[1024];
    vsprintf(buff, format, args);
    va_end(args);
    write(buff);
    return strlen(buff);
}

u32 kprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buff[1024];
    vsprintf(buff, format, args);
    va_end(args);
    console_print(buff);
    return strlen(buff);
}

u32 sprintf(char *buff, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsprintf(buff, format, args);
    va_end(args);
    return strlen(buff);
}


