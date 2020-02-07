[org 0x7c00]
    KERNEL_OFFSET equ 0x1000
    mov [BOOT_DRIVE], dl
    mov bp, 0x9000
    mov sp, bp

    mov bx, MSG_REAL
    call print

    call load_kernel
    call switch_to_pm
    jmp $
%include "src/real/print.asm"
%include "src/disk/load.asm"
%include "src/protect/gdt.asm"
%include "src/protect/print.asm"
%include "src/protect/switch.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD
    call print

    mov bx, KERNEL_OFFSET
    mov dh, 2
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret
[bits 32]
begin_pm:
    mov ebx, MSG_PROT
    call print_string_pm
    call KERNEL_OFFSET
    jmp $

BOOT_DRIVE:
    db 0
MSG_REAL:
    db "REAL MODE", 0x0a, 0
MSG_PROT:
    db "PROT MODE", 0x0a, 0
MSG_LOAD:
    db "LOAD MODE", 0x0a, 0

    times 510 - ($ - $$) db 0
    dw 0xaa55
