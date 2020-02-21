[global gdt_flush]
gdt_flush:
    ; gdt 描述 的在栈中
    mov eax, [esp + 4]
    ; esp + 4 第一个参数
    ; esp 返回后执行的下一行地址
    lgdt [eax]

    ; 数据段
    ;跳转代码, cs 自动设置微0x08
    mov ax, 0x10;
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08: flush

flush:
    ret
