run: os.img
	@qemu-system-i386 -fda $<

os.img: boot.bin kernel.bin
	@cat $^ > $@

kernel.bin: kernel_entry.o kernel.o x86.o string.o idt.o isr.o interrupt.o clock.o irq.o
	@i686-elf-ld -o $@ -Ttext 0x9000 $^ --oformat binary

boot.bin: boot/boot.asm
	@nasm $< -f bin -o $@

x86.o: libs/x86.c
	@i686-elf-gcc -g -ffreestanding -c $< -o $@

string.o: libs/string.c
	@i686-elf-gcc -g -ffreestanding -c $< -o $@

idt.o: trap/idt.c
	@i686-elf-gcc -g -ffreestanding -c $< -o $@

isr.o: trap/isr.c
	@i686-elf-gcc -g -ffreestanding -c $< -o $@

irq.o: driver/irq.c
	@i686-elf-gcc -g -ffreestanding -c $< -o $@

clock.o: driver/clock.c
	@i686-elf-gcc -g -ffreestanding -c $< -o $@

kernel.o: kernel/kernel.c
	 @i686-elf-gcc -g -ffreestanding -c $< -o $@

kernel_entry.o: boot/kernel_entry.asm
	@nasm $< -f elf -o $@

interrupt.o: trap/interrupt.asm
	@nasm $< -f elf -o $@
	
clean:
	rm -rf *.bin *.img *.o

