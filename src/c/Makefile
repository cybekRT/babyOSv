all: kernel.bin

kmain.o: kmain.c
	gcc -m32 -c $< -o $@

amain.o: amain.asm
	nasm -felf $< -o $@

kernel.bin: kmain.o amain.o
	/usr/local/Cellar/x86_64-elf-binutils/2.34/bin/x86_64-elf-ld -m elf_i386 -T linker.ld $^ -o $@

clean:
	rm kmain.o amain.o kernel.bin || true
