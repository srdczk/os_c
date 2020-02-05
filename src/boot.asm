; A simple boot sector 
org 0x7c00 ;load from 0x7c00

entry:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov si, msg

putloop:
    mov al, [si]
    add si, 1
    cmp al, 0
    je fin
    mov ah, 0x0e
    mov bx, 15
    int 0x10
    jmp putloop

fin:
    HLT
    jmp fin

msg:
    db 0x0a, 0x0a
    db "NIMASILE"
    db 0xa
    db 0

times 510-($-$$) db 0


dw 0xaa55
