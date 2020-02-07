disk_load:
    pusha
    ; 保存所有寄存器
    push dx

    ; bios int0x13 中断
    mov ah, 0x02
    mov al, dh
    ; 读入的扇区号
    mov cl, 0x02
    ; 柱面号 以及磁头号
    mov ch, 0x00
    mov dh, 0x00

    int 0x13
    jc disk_error ;if error

    pop dx
    cmp al, dh
    jne sectors_error
    popa
    ret

disk_error:
    mov bx, DISK_ERROR
    call print
    jmp disk_loop

sectors_error:
    mov bx, SECTORS_ERROR
    call print

disk_loop:
    jmp disk_loop

DISK_ERROR:
    db "Disk error", 0
SECTORS_ERROR:
    db "Sectors error", 0
