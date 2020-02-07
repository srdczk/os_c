[bits 16]
switch_to_pm:
    cli
    lgdt [gdt_descriptor]

    ; 打开 20 总线
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    ; 跳转 到 代码段 : init_pm
    ; 段选择子 + 段内偏移, 此处 数据段 和代码段 都是 0 起始
    jmp CODE_SEG:init_pm
    ; 初始化保护模式
[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; 初始化栈
    mov ebp, 0x90000
    mov esp, ebp

    call begin_pm

