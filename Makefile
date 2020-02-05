run: build qemu

build:
	@nasm src/boot.asm -f bin -o boot.bin

qemu:
	@qemu-system-i386 boot.bin

clean:
	rm -rf boot.bin
