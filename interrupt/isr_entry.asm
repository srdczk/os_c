[extern int_dispatch]
all_int:
    pusha 
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp ;指针
    call int_dispatch
    add esp, 4

    pop ebx 
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    mov ss, bx 

    popa
    add esp, 8  ; 错误码
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
    cli
    push byte 0
    push byte 0
    jmp all_int
isr1:
    cli
    push byte 0
    push byte 1
    jmp all_int

; 2: Non Maskable Interrupt Exception
isr2:
    cli
    push byte 0
    push byte 2
    jmp all_int

; 3: Int 3 Exception
isr3:
    cli
    push byte 0
    push byte 3
    jmp all_int

; 4: INTO Exception
isr4:
    cli
    push byte 0
    push byte 4
    jmp all_int

; 5: Out of Bounds Exception
isr5:
    cli
    push byte 0
    push byte 5
    jmp all_int

; 6: Invalid Opcode Exception
isr6:
    cli
    push byte 0
    push byte 6
    jmp all_int

; 7: Coprocessor Not Available Exception
isr7:
    cli
    push byte 0
    push byte 7
    jmp all_int

; 8: Double Fault Exception (With Error Code!)
isr8:
    cli
    push byte 8
    jmp all_int

; 9: Coprocessor Segment Overrun Exception
isr9:
    cli
    push byte 0
    push byte 9
    jmp all_int
    
; 10: Bad TSS Exception (With Error Code!)
isr10:
    cli
    push byte 10
    jmp all_int

; 11: Segment Not Present Exception (With Error Code!)
isr11:
    cli
    push byte 11
    jmp all_int

; 12: Stack Fault Exception (With Error Code!)
isr12:
    cli
    push byte 12
    jmp all_int

; 13: General Protection Fault Exception (With Error Code!)
isr13:
    cli
    push byte 13
    jmp all_int

; 14: Page Fault Exception (With Error Code!)
isr14:
    cli
    push byte 14
    jmp all_int

; 15: Reserved Exception
isr15:
    cli
    push byte 0
    push byte 15
    jmp all_int

; 16: Floating Point Exception
isr16:
    cli
    push byte 0
    push byte 16
    jmp all_int

; 17: Alignment Check Exception
isr17:
    cli
    push byte 0
    push byte 17
    jmp all_int

; 18: Machine Check Exception
isr18:
    cli
    push byte 0
    push byte 18
    jmp all_int

; 19: Reserved
isr19:
    cli
    push byte 0
    push byte 19
    jmp all_int

; 20: Reserved
isr20:
    cli
    push byte 0
    push byte 20
    jmp all_int

; 21: Reserved
isr21:
    cli
    push byte 0
    push byte 21
    jmp all_int

; 22: Reserved
isr22:
    cli
    push byte 0
    push byte 22
    jmp all_int

; 23: Reserved
isr23:
    cli
    push byte 0
    push byte 23
    jmp all_int

; 24: Reserved
isr24:
    cli
    push byte 0
    push byte 24
    jmp all_int

; 25: Reserved
isr25:
    cli
    push byte 0
    push byte 25
    jmp all_int

; 26: Reserved
isr26:
    cli
    push byte 0
    push byte 26
    jmp all_int

; 27: Reserved
isr27:
    cli
    push byte 0
    push byte 27
    jmp all_int

; 28: Reserved
isr28:
    cli
    push byte 0
    push byte 28
    jmp all_int

; 29: Reserved
isr29:
    cli
    push byte 0
    push byte 29
    jmp all_int

; 30: Reserved
isr30:
    cli
    push byte 0
    push byte 30
    jmp all_int

; 31: Reserved
isr31:
    cli
    push byte 0
    push byte 31
    jmp all_int

irq0:
    cli
    push byte 0
    push byte 32
    jmp all_int
irq1:
	cli
	push byte 1
	push byte 33
    jmp all_int 
irq2:
	cli
	push byte 2
	push byte 34
	jmp all_int 

irq3:
	cli
	push byte 3
	push byte 35
irq4:
	cli
	push byte 4
	push byte 36
	jmp all_int 

irq5:
	cli
	push byte 5
	push byte 37
    jmp all_int

irq6:
	cli
	push byte 6
	push byte 38
    jmp all_int

irq7:
	cli
	push byte 7
	push byte 39
    jmp all_int

irq8:
	cli
	push byte 8
	push byte 40
    jmp all_int 

irq9:
	cli
	push byte 9
	push byte 41
    jmp all_int 

irq10:
	cli
	push byte 10
	push byte 42
    jmp all_int 

irq11:
	cli
	push byte 11
	push byte 43
    jmp all_int

irq12:
	cli
	push byte 12
	push byte 44
    jmp all_int 

irq13:
	cli
	push byte 13
	push byte 45
    jmp all_int

irq14:
	cli
	push byte 14
	push byte 46
    jmp all_int 

irq15:
	cli
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

