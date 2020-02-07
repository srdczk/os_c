[bits 32]
;print string directly to video memory
; 0xb8000 + 2 * (row * 80 + col)
VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

print_string_pm
    pusha
    mov edx, VIDEO_MEMORY

print_string_pm_loop:
    mov al, [ebx]
    mov ah, WHITE_ON_BLACK

    cmp al, 0
    je done

    mov [edx], ax ; al 存进[edx] -> 显存 + 2 --> 列数 + 2

    add ebx, 1
    add edx, 2

    jmp print_string_pm_loop
done:
    popa
    ret
; should init GDT and enter protected mode

