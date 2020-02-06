[ORG 0x7c00]
   xor ax, ax ; make it zero
   mov ds, ax
   mov si, msg
put_char:
    mov al, [si]
    add si, 1
    or al, al
    jz loop
    mov ah, 0x0e
    mov bx, 0x00
    int 0x10
   jmp put_char
 
loop:
   jmp loop
 
msg:
    db "NIMASILE", 0x0d, 0x0a, 0x00
    times 510 - ($ - $$) db 0
    dw 0xaa55
