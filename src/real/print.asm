print_done:
    ret
print:
    mov al, [bx]
    add bx, 1
    cmp al, 0
    je print_done
    mov ah, 0x0e
    int 0x10
    jmp print

