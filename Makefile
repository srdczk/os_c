
C_SRCS = $(shell find . -name "*.c")
C_OBJS = $(patsubst %.c, %.o, $(C_SRCS))
ASM_SRCS = $(shell find . -name "*.asm")
ASM_OBJS = $(patsubst %.asm, %.o, $(ASM_SRCS))


CC = gcc
LD = ld
ASM = nasm

C_FLAGS = -c -Wall -m32 -ggdb -gstabs+ -nostdinc -fno-pic  -fno-builtin -fno-stack-protector -I include
LD_FLAGS = -T linker/linker.ld -m elf_i386 -nostdlib
ASM_FLAGS = -f elf -g -F stabs

run: $(ASM_OBJS) $(C_OBJS) link init_disk fdisk qemu

%.o: %.c
	$(CC) $(C_FLAGS) $< -o $@

%.o: %.asm
	$(ASM) $(ASM_FLAGS) $<

link:
	$(LD) $(LD_FLAGS) $(ASM_OBJS) $(C_OBJS) -o zl_kernel

init_disk:
	dd if=/dev/zero of=disk.img bs=10M count=8 conv=notrunc

fdisk:
	fdisk disk.img < fdisk.txt
	
user_prog:
	ld -e umain -m elf_i386 -nostdlib disk_pro.o -o disk_pro

clean:
	rm -rf $(ASM_OBJS) $(C_OBJS) zl_kernel *.img

qemu:
	qemu-system-i386 -hda disk.img -m 512M -kernel zl_kernel

gdb:
	qemu-system-i386 -S -s -hda disk.img -m 512M -kernel zl_kernel &
	sleep 1
	gdb -x gdbinit
