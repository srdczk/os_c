; a simple boot sector
org 0x7c00

mov ah, 0x0e ;tty mode
mov al, 'n'
int 0x10
mov al, 'a'
int 0x10
mov al, 's'
int 0x10
mov al, 'i'
int 0x10
int 0x10
mov al, 0x0a
int 0x10

jmp $  ;jump to cur addr -> loop

times 510 - ($ - $$) db 0

db 0x55
db 0xaa

;dw 0xaa55
; 由于是小端 机器 , 所以 0xaa55 存储 -> 55 aa
