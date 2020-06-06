
NASM		= nasm -Iinc/ -Isrc/

ifeq ($(OS),Windows_NT)
	#BOCHS		= bochs -f bochs.cfg
	BOCHS		= d:\Programs\Bochs\bochsdbg-p4-smp.exe -f bochs-win.cfg
	OUT		= $(PWD)/out
	GCC		= i386-elf-gcc
	# /usr/local/osdev/bin/i386-elf-gcc
	LD		= i386-elf-ld
	# /usr/local/osdev/bin/i386-elf-ld
	QEMU		= D:\Programs\Qemu\qemu-system-i386.exe
	DD		= D:\Programs\Cygwin\bin\dd
	# qemu-system-i386
	PCEM		= D:\Programs\PCem\PCem.exe
else
	BOCHS		= bochs -f bochs.cfg
	OUT		= $(PWD)/out
	GCC		= /usr/local/osdev/bin/i386-elf-gcc
	LD		= /usr/local/osdev/bin/i386-elf-ld
	QEMU		= qemu-system-i386
	DD		= dd
	PCEM		= 
endif

all: floppy.img

#floppy.bin: $(OUT)/boot.bin $(OUT)/kernel.bin
#	cat $^ > $@

floppy.img: out/boot1.bin out/boot2.bin out/kernel.bin
	cat $^ out/boot1.bin > $@
	$(DD) if=/dev/zero of=$@ bs=1 count=0 seek=1474560

out/boot1.bin: src/boot1.asm out/boot2.bin
	$(NASM) $< -o $@ -fbin -DBOOT2_SIZE=$(strip $(shell wc -c < out/boot2.bin))

out/boot2.bin: src/boot2.asm out/kernel.bin
	$(NASM) $< -fbin -o $@ -l out/kernel.lst -DKERNEL_SIZE=$(strip $(shell wc -c < out/kernel.bin))

out/kernel.bin: out/kmain.o out/kmain_startup.o
	$(LD) -nostdlib -nolibc -nostartfiles -nodefaultlibs -m elf_i386 -T src/linker.ld $^ -o $@

out/kmain.o: src/kmain.cpp
	$(GCC) -Iinc/ -s -O2 -m32 -c $< -o $@ 

out/kmain_startup.o: src/kmain_startup.asm
	$(NASM) -felf $< -o $@

clean:
	rm out/* floppy.img || true
#	$(MAKE) -C src clean

qemu: floppy.img
	$(QEMU) -fda $< -boot ac -m 32 -monitor stdio

bochs: floppy.img
	$(BOCHS) -q

pcem: floppy.img
	$(PCEM)
