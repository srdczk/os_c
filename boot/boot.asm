

; GRUB multiboot 规范

MBOOT_HEADER_MAGIC  equ 0x1badb002

MBOOT_PAGE_ALIGN    equ 1 << 0 ; 引导模块安装 4K 对齐
MBOOT_MEM_INFO      equ 1 << 1 ; 让 GRUB 把 内存空间信息包含在 信息结构中

MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN

MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)


[bits 32]

section .text
; 代码段开始

dd MBOOT_HEADER_MAGIC
dd MBOOT_HEADER_FLAGS
dd MBOOT_CHECKSUM

; 内核代码入口 -> 在 ld 链接脚本中声明
[global start]
[global glb_mboot_ptr] ;multiboot 变量
[extern main] ; kernel c 语言函数入口

start:
    ; 关中断
    cli

    mov esp, STACK_TOP
    mov ebp, 0
    and esp, 0xfffffff0 ; 16 字节对齐
    mov [glb_mboot_ptr], ebx
    call main

loop:
    hlt
    jmp loop


section .bss ; 数据段
stack:
; 作为内核栈
    ; 将 glb_multiboot_入栈
    resb 32768
glb_mboot_ptr:
    resb 4
STACK_TOP   equ $ - stack - 1

