; 寄存器的恢复 和保存
[global switch_to]
switch_to:
    ; 将 第一个参数 -> pre->context地址 和保存
    ; 利用 pushf 最后一个入栈 的是eflags 设置eflags
    mov eax, [esp + 4]
    
    mov [eax], esp
    mov [eax + 4], ebp
    mov [eax + 8], ebx
    mov [eax + 12], esi
    mov [eax + 16], edi
    pushf
    pop ecx
    mov [eax + 20], ecx
    ; 将 正在运行的寄存器设置为第二个参数的值
    mov eax, [esp + 8]
    
    mov esp, [eax]
    mov ebp, [eax + 4]
    mov ebx, [eax + 8]
    mov esi, [eax + 12]
    mov edi, [eax + 16]
    ; elags 保存
    mov eax, [eax + 20]
    push eax
    ; 恢复eflags
    popf

    ret
