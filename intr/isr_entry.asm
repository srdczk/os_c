[extern int_dispatch]
all_int:

    push ds
    push es
    push fs
    push gs
    pusha

    mov eax, 0x10
    mov ds, ax
    mov es, ax

    push esp ;指针
    call int_dispatch
    add esp, 4
    ;中断退出函数, 伪造中断现场的时候使用
    popa
[global int_exit]
int_exit:
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 8
    iret 

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

isr0:

    push byte 0
    push byte 0
    jmp all_int
isr1:

    push byte 0
    push byte 1
    jmp all_int

; 2: Non Maskable Interrupt Exception
isr2:

    push byte 0
    push byte 2
    jmp all_int

; 3: Int 3 Exception
isr3:

    push byte 0
    push byte 3
    jmp all_int

; 4: INTO Exception
isr4:

    push byte 0
    push byte 4
    jmp all_int

; 5: Out of Bounds Exception
isr5:

    push byte 0
    push byte 5
    jmp all_int

; 6: Invalid Opcode Exception
isr6:

    push byte 0
    push byte 6
    jmp all_int

; 7: Coprocessor Not Available Exception
isr7:

    push byte 0
    push byte 7
    jmp all_int

; 8: Double Fault Exception (With Error Code!)
isr8:

    push byte 8
    jmp all_int

; 9: Coprocessor Segment Overrun Exception
isr9:

    push byte 0
    push byte 9
    jmp all_int
    
; 10: Bad TSS Exception (With Error Code!)
isr10:

    push byte 10
    jmp all_int

; 11: Segment Not Present Exception (With Error Code!)
isr11:

    push byte 11
    jmp all_int

; 12: Stack Fault Exception (With Error Code!)
isr12:

    push byte 12
    jmp all_int

; 13: General Protection Fault Exception (With Error Code!)
isr13:

    push byte 13
    jmp all_int

; 14: Page Fault Exception (With Error Code!)
isr14:

    push byte 14
    jmp all_int

; 15: Reserved Exception
isr15:

    push byte 0
    push byte 15
    jmp all_int

; 16: Floating Point Exception
isr16:

    push byte 0
    push byte 16
    jmp all_int

; 17: Alignment Check Exception
isr17:

    push byte 0
    push byte 17
    jmp all_int

; 18: Machine Check Exception
isr18:

    push byte 0
    push byte 18
    jmp all_int

; 19: Reserved
isr19:

    push byte 0
    push byte 19
    jmp all_int

; 20: Reserved
isr20:

    push byte 0
    push byte 20
    jmp all_int

; 21: Reserved
isr21:

    push byte 0
    push byte 21
    jmp all_int

; 22: Reserved
isr22:

    push byte 0
    push byte 22
    jmp all_int

; 23: Reserved
isr23:

    push byte 0
    push byte 23
    jmp all_int

; 24: Reserved
isr24:

    push byte 0
    push byte 24
    jmp all_int

; 25: Reserved
isr25:

    push byte 0
    push byte 25
    jmp all_int

; 26: Reserved
isr26:

    push byte 0
    push byte 26
    jmp all_int

; 27: Reserved
isr27:

    push byte 0
    push byte 27
    jmp all_int

; 28: Reserved
isr28:

    push byte 0
    push byte 28
    jmp all_int

; 29: Reserved
isr29:

    push byte 0
    push byte 29
    jmp all_int

; 30: Reserved
isr30:

    push byte 0
    push byte 30
    jmp all_int

; 31: Reserved
isr31:

    push byte 0
    push byte 31
    jmp all_int

irq0:

    push byte 0
    push byte 32
    jmp all_int
irq1:

	push byte 1
	push byte 33
    jmp all_int 
irq2:

	push byte 2
	push byte 34
	jmp all_int 

irq3:

	push byte 3
	push byte 35
irq4:

	push byte 4
	push byte 36
	jmp all_int 

irq5:

	push byte 5
	push byte 37
    jmp all_int

irq6:

	push byte 6
	push byte 38
    jmp all_int

irq7:

	push byte 7
	push byte 39
    jmp all_int

irq8:

	push byte 8
	push byte 40
    jmp all_int 

irq9:

	push byte 9
	push byte 41
    jmp all_int 

irq10:

	push byte 10
	push byte 42
    jmp all_int 

irq11:

	push byte 11
	push byte 43
    jmp all_int

irq12:

	push byte 12
	push byte 44
    jmp all_int

irq13:

	push byte 13
	push byte 45
    jmp all_int

irq14:

	push byte 14
	push byte 46
    jmp all_int 

irq15:

	push byte 15
	push byte 47
    jmp all_int


[global isrs]
isrs:
    dd isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, isr9, isr10
    dd isr11, isr12, isr13, isr14, isr15, isr16, isr17
    dd isr18, isr19, isr20, isr21, isr22, isr23, isr24
    dd isr25, isr26, isr27, isr28, isr29, isr20, isr31
    dd irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7, irq8, irq9, irq10
    dd irq11, irq12, irq13, irq14, irq15

