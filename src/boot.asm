; boot.asm
   mov ax, 0x07c0
   mov ds, ax
; 从 0x7c00 处开始 
   mov si, msg
   cld
put_char:
   or al, al ; if al == 0 end
   jz loop   
   mov ah, 0x0e
   mov bh, 0
   mov al, [si]
   add si, 1
   int 0x10
   jmp put_char
 
loop:
   jmp loop
 
msg:    
    db "NIAMSILE", 0x0d, 0x0a, 0x00
    times 510-($-$$) db 0
    dw 0xaa55
