[org 0x7c00]

KERNEL_OFFSET equ 0x1000

; bi:os set dl -> boot_drive

    mov [BOOT_DRIVE], dl
    mov bp, 0x9000
    mov sp, bp

    mov bx, MSG_REAL
    call print
    call print_nl
    
    call load_kernel
    ; switch_to_pm -> begin_pm
    call switch_to_pm

    jmp $
%include "src/boot/print.asm"
%include "src/boot/gdt.asm"
%include "src/boot/switch.asm"
%include "src/boot/disk.asm"
%include "src/boot/protect_print.asm"
[bits 16]
load_kernel:
    mov bx, MSG_LOAD
    call print
    call print_nl

    mov bx, KERNEL_OFFSET
    ;after kernel is large -> make it big
    mov dh, 16
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

[bits 32]
begin_pm:
    mov ebx, MSG_PROT
    ; kernel 已经被加载 到 0x1000 处
    call print_string_pm
    call KERNEL_OFFSET
    jmp $

BOOT_DRIVE:
    db 0
MSG_REAL:
    db "REAL MODE", 0
MSG_PROT:
    db "PROT MODE", 0
MSG_LOAD:
    db "LOAD MODE", 0

    times 510 - ($ - $$) db 0

    dw 0xaa55
