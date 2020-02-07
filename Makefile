run: os.img
	@qemu-system-i386 -fda $<

os.img: boot.bin kernel.bin
	@cat $^ > $@

kernel.bin: kernel_entry.o kernel.o port.o
	@i686-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

boot.bin: src/boot/boot.asm
	@nasm $< -f bin -o $@

port.o: src/driver/port.c
	@i686-elf-gcc -g -ffreestanding -c $< -o $@

kernel.o: src/kernel/kernel.c
	 @i686-elf-gcc -g -ffreestanding -c $< -o $@

kernel_entry.o: src/boot/kernel_entry.asm
	@nasm $< -f elf -o $@
	
clean:
	rm -rf *.bin *.img *.o

