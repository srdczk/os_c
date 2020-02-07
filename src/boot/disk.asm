disk_load:
    pusha
    push dx

    ; 读取到内存中的地址 ---> 存放在bx 中
    mov ah, 0x02
    ; 读取的扇区数
    mov al, dh
    ; 开始读取的扇区编号
    mov cl, 0x02 
    mov ch, 0x00
    
    mov dh, 0x00

    int 0x13
    jc disk_error

    pop dx
    cmp al, dh
    jne sectors_error
    popa
    ret

disk_error:
    mov bx, DISK_ERROR
    call print
    call print_nl
    jmp disk_loop

sectors_error:
    mov bx, SECTORS_ERROR
    call print
    call print_nl

disk_loop:
    jmp disk_loop

DISK_ERROR:
    db "DISK ERROR", 0
SECTORS_ERROR:
    db "SECTORS ERROR", 0
