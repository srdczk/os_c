[ORG 0x7c00]
    mov bp, 0x9000 ; set stack
    mov sp, bp
    
    mov bx, MSG_REAL
    call print

    call switch_to_pm

    jmp $ ;never executed

%include "src/real/print.asm"
%include "src/protect/gdt.asm"
%include "src/protect/print.asm"
%include "src/protect/switch.asm"

; enter the ptotected mode
[bits 32]
begin_pm:
    mov ebx, MSG_PROT
    call print_string_pm
    jmp $

; 实模式 显示:
MSG_REAL:
    db "REALP", 0
MSG_PROT:
    db "KOBED", 0

    times 510 - ($ - $$) db 0
    dw 0xaa55

