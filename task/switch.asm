[bits 32]
[global switch_to]

switch_to:
    ; 保存现场
    push esi
    push edi
    push ebx
    push ebp

    mov eax, [esp + 20] ; 栈中第一个参数 + 4 + 4 * 4
    mov [eax], esp

    ;恢复成下一个现场
    mov eax, [esp + 24]
    mov esp, [eax]

    pop ebp
    pop ebx
    pop edi
    pop esi
    ret


