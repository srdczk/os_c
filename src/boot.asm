%macro Print 1
    mov si, %1
put_char:
    mov al, [si]
    add si, 1
    or al, al
    jz done
    mov ah, 0x0e
    mov bh, 0x00
    int 0x10
    jmp put_char
done:
    ret
%endmacro
[ORG 0x7c00]
    xor ax, ax
    mov ds, ax
    Print msg
loop:
    jmp loop

msg: 
    db "NIMASILE", 0x0d, 0x0a, 0x00

    times 510 - ($ - $$) db 0
    dw 0xaa55
        
