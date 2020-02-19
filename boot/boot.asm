[org 0x7c00]

load_addr equ 0x9000

; bios set dl -> boot_drive

    mov [boot_drive], dl
    mov bp, 0x9000
    mov sp, bp
    call read_floppy
    ; switch_to_pm -> begin_pm
    ; 开启保护模式
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp code_seg: begin_pm
[bits 16]
read_floppy:
    mov bx, load_addr
    ;after kernel is large -> make it big
    mov dh, 16
    mov dl, [boot_drive]
    pusha
    push dx
    mov ah, 0x02
    mov al, dh
    mov cl, 0x02
    mov ch, 0x00
    mov dh, 0x00
    int 0x13
    jc read_error
    pop dx
    popa
    ret

read_error:
    hlt
    jmp read_error

[bits 32]
begin_pm:
    ; kernel 已经被加载 到 0x1000 处
    mov ax, data_seg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov esp, 0x9000
    mov ebp, esp
    call load_addr
    jmp $

boot_drive:
    db 0
gdt_start: ; don't remove the labels, they're needed to compute sizes and jumps

gdt_null:
    dd 0x0 ; 4 byte
    dd 0x0 ; 4 byte

; 重新设置 gdt -> 
gdt_code: 
    dw 0xffff
    dw 0x0
    db 0x0
    db 0x9a
    db 0xcf
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 0x92
    db 0xcf
    db 0x0
gdt_user_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 0xfa
    db 0xcf
    db 0x0
gdt_user_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 0xf2
    db 0xcf
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size (16 bit), always one less of its true size
    dd gdt_start ; address (32 bit)

code_seg equ gdt_code - gdt_start
data_seg equ gdt_data - gdt_start
user_code_seg equ gdt_user_code - gdt_start
user_data_seg equ gdt_user_data - gdt_start

    times 510 - ($ - $$) db 0

    dw 0xaa55
