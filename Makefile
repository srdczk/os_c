

# 定义所有的 c -> o, asm -> o

C_SRCS = $(shell find . -name "*.c")
# .o 替换 .c
C_OBJS = $(patsubst %.c, %.o, $(C_SRCS))
ASM_SRCS = $(shell find . -name "*.asm")
ASM_OBJS = $(patsubst %.asm, %.o, $(ASM_SRCS))

CC = gcc
LD = ld
ASM = nasm

C_FLAGS = -c -Wall -m32 -ggdb -gstabs+ -nostdinc -fno-builtin -fno-stack-protector -I include
LD_FLAGS = -T linker/linker.ld -m elf_i386 -nostdlib
ASM_FLAGS = -f elf -g -F stabs

run:$(C_OBJS) $(ASM_OBJS) link update_image qemu

%.o: %.c
	$(CC) $(C_FLAGS) $< -o $@

%.o: %.asm
	$(ASM) $(ASM_FLAGS) $<

link:
	$(LD) $(LD_FLAGS) $(ASM_OBJS) $(C_OBJS) -o zl_kernel

clean:
	rm -rf $(ASM_OBJS) $(C_OBJS) zl_kernel

update_image:
	sudo mount floppy.img /mnt
	sudo cp zl_kernel /mnt/zl_kernel
	sleep 1
	sudo umount /mnt

qemu:
	qemu-system-i386 -fda floppy.img
