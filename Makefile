all:run

kernel.bin: kernel_entry.o kernel.o
	@i686-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

kernel_entry.o: src/kernel/kernel_entry.asm
	@nasm $< -f elf -o $@

kernel.o: src/kernel/kernel.c
	@i686-elf-gcc -ffreestanding -c $< -o $@

kernel.dis: kernel.bin
	@ndisasm -b 32 $< > $@

boot.bin: src/boot.asm
	@nasm $< -f bin -o $@

os.img: boot.bin kernel.bin
	cat $^ > $@

run: os.img
	@qemu-system-i386 -fda $<

clean:
	rm -rf *.bin *.o *.dis *.img
