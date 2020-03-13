#include "../include/keyboard.h"
#include "../include/irq.h"

#define IRQ_KEYBOARD 0x21

io_buffer kb_buffer;

//转义字符
#define esc '\033'
#define backspace '\b'
#define tab '\t'
#define enter '\n'
#define delete '\177'

// 不可见
#define char_invisible 0
#define ctrl_l_char char_invisible
#define ctrl_r_char char_invisible
#define shift_l_char char_invisible
#define shift_r_char char_invisible
#define alt_l_char char_invisible
#define alt_r_char char_invisible
#define caps_lock_char char_invisible

//控制字符
#define shift_l_make 0x2a
#define shift_r_make 0x36
#define alt_l_make 0x38
#define alt_r_make 0xe038
#define alt_r_break 0xe0b8
#define ctrl_l_make 0x1d
#define ctrl_r_make 0xe01d
#define ctrl_r_break 0xe09d
#define caps_lock_make 0x3a

// 是否按下
static u8 ctrl_status = 0, shift_status = 0, alt_status = 0, caps_lock_status = 0, ext_scan_code = 0;

// 对应的大写小写字符表
static char keymap[][2] = {
    {0, 0},
    {esc, esc},
    {1, '!'},
    {2, '@'},
    {3, '#'},
    {4, '$'},
    {5, '%'},
    {6, '^'},
    {7, '&'},
    {8, '*'},
    {9, '('},
    {0, ')'},
    {'-', '_'},
    {'=', '+'},
    {backspace, backspace},
    {tab, tab},
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {enter, enter},
    {ctrl_l_char, ctrl_l_char},
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'},
    {'\'', '"'},
    {'`', '~'},
    {shift_l_char, shift_l_char},
    {'\\', '|'},
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
    {shift_r_char, shift_r_char},
    {'*', '*'},
    {alt_l_char, alt_l_char},
    {' ', ' '},
    {caps_lock_char, caps_lock_char}
};

void keyboard_init() {
    irq_enable(IRQ_KEYBOARD);
}

void kbuffer_init() {
    buffer_init(&kb_buffer);
}

void keyboard_handler(u8 code) {
    u16 handle_code = 0xff & code;
    if (!(code ^ 0xe0)) {
        ext_scan_code = 1;
        return;
    }
    if (ext_scan_code) {
        handle_code |= 0xe000;
        ext_scan_code = 0;
    }
    
    u16 is_break = handle_code & 0x80;
    if (is_break) {
        // 获得通码
        u16 make = (handle_code & 0xff7f);
        if (!(make ^ ctrl_l_make) || !(make ^ ctrl_r_make)) ctrl_status = 0;
        else if (!(make ^ shift_l_make) || !(make ^ shift_r_make)) shift_status = 0;
        else if (!(make ^ alt_l_make) || !(make ^ alt_r_make)) alt_status = 0;
        return;
    }
    if ((!handle_code || handle_code > 0x3a) &&\
        (handle_code != alt_r_make && handle_code != ctrl_r_make)) {
        console_print("Unknow\n");
        return;
    }
    u8 shift = 0;
    // 如果 是对shift相关的
      if (handle_code < 0x0e || handle_code == 0x29 || handle_code == 0x1a ||\
          handle_code == 0x1b || handle_code == 0x2b || handle_code == 0x27 ||\
          handle_code == 0x28 || handle_code == 0x33 || handle_code == 0x34 || handle_code == 0x35) {
          shift = shift_status;
      } else if (shift_status || caps_lock_status) shift = 1;
      else shift = 0;

      u8 index = (handle_code & 0xff);
      char put = keymap[index][shift];

      if (ctrl_status && (put == 'l' || put == 'u')) put -= 'a';

      if (!buffer_full(&kb_buffer)) {
          buffer_putchar(&kb_buffer, put);
      }

      if (!(handle_code ^ ctrl_l_make) || !(handle_code ^ ctrl_r_make)) ctrl_status = 1;
      else if (!(handle_code ^ shift_l_make) || !(handle_code ^ shift_r_make)) shift_status = 1;
      else if (!(handle_code ^ alt_l_make) || !(handle_code ^ alt_r_make)) alt_status = 1;
      else if (!(handle_code ^ caps_lock_make)) caps_lock_status = !caps_lock_status;

}
