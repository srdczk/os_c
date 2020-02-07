[bits 16]
switch_to_pm:
; 切换到保护模式
; 关闭中断
    cli
    lgdt [gdt_descriptor]
; 打开 A20 地址总线
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm
; 32 位保护模式的基础设置
[bits 32]
; 段寄存器 设置 数据段, 栈寄存器 设置 0x90000
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov ebp, 0x90000
    mov esp, ebp

    call begin_pm
