
; 符合 multiboot 规范 


[bits 32]

section .init.text

dd 0x1badb002 ; 魔数

dd 0x00000003 ; flag 表示 multiboot 读取 内存信息 4KB 对齐

dd 0xe4524ffb ; checksum + flag + migic = 0

[global start]
[global tmp_multiboot]
[extern main]

start:
    cli ;关中断 -> gdt 还未真正设置
    mov [tmp_multiboot], ebx
    mov esp, stack_top
    and esp, 0xfffffff0; 16 字节对齐
    xor ebp, ebp

    call main

section .init.data

stack:
    times 1024 db 0

stack_top equ $ - stack - 1

tmp_multiboot:
    dd 0


