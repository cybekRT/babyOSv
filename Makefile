NASM		= nasm -Isrc/
BOCHS		= bochs -f bochs.cfg
OUT		= $(PWD)/out

all: floppy.bin

#floppy.bin: $(OUT)/boot.bin $(OUT)/kernel.bin
#	cat $^ > $@

floppy.bin: out/boot1.bin out/boot2.bin out/kernel.bin
	cat $^ > $@

out/boot1.bin: src/boot1.asm out/boot2.bin
	$(NASM) $< -o $@ -fbin -DBOOT2_SIZE=$(strip $(shell wc -c < out/boot2.bin))

out/boot2.bin: src/boot2.asm out/kernel.bin
	$(NASM) $< -fbin -o $@ -l out/kernel.lst -DKERNEL_SIZE=$(strip $(shell wc -c < out/kernel.bin))

out/kernel.bin: out/kmain.o out/kmain_startup.o
	/usr/local/osdev/bin/i386-elf-ld -nostdlib -nolibc -nostartfiles -nodefaultlibs -m elf_i386 -T src/linker.ld $^ -o $@

out/kmain.o: src/kmain.c
	/usr/local/osdev/bin/i386-elf-gcc -s -O2 -m32 -c $< -o $@ 

out/kmain_startup.o: src/kmain_startup.asm
	nasm -felf $< -o $@

clean:
	rm out/* floppy.bin || true
#	$(MAKE) -C src clean

qemu: floppy.bin
	qemu-system-i386 -fda $< -boot ac -m 32 -monitor stdio

bochs: floppy.bin
	$(BOCHS) -q
