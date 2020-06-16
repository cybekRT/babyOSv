
NASM		= nasm -Iboot/

ifeq ($(OS),Windows_NT)
	BOCHS		= D:\Programs\Bochs\bochsdbg-p4-smp.exe -f bochs-win.cfg
	OUT		= $(PWD)/out
	GCC_PREFIX	= i386-elf-
	QEMU		= D:\Programs\Qemu\qemu-system-i386.exe
	DD		= D:\Programs\Cygwin\bin\dd
	# qemu-system-i386
	PCEM		= D:\Programs\PCem\PCem.exe
else
	BOCHS		= bochs -f bochs.cfg
	OUT		= $(PWD)/out
	GCC_PREFIX	= /usr/local/osdev/bin/i386-elf-
	QEMU		= qemu-system-i386
	DD		= dd
	PCEM		= 
endif

GCC			= $(GCC_PREFIX)gcc
LD			= $(GCC_PREFIX)ld
GCC_FLAGS		= -fno-isolate-erroneous-paths-attribute -fno-asynchronous-unwind-tables

all: floppy.img

floppy.img: out/boot1.bin out/boot2.bin out/kernel.bin
	cat $^ out/boot1.bin > $@
	$(DD) if=/dev/zero of=$@ bs=1 count=0 seek=1474560

out/boot1.bin: boot/boot1.asm out/boot2.bin
	$(NASM) $< -o $@ -fbin -DBOOT2_SIZE=$(strip $(shell wc -c < out/boot2.bin))

out/boot2.bin: boot/boot2.asm out/kernel.bin
	$(NASM) $< -fbin -o $@ -DKERNEL_SIZE=$(strip $(shell wc -c < out/kernel.bin))

out/kernel.elf: out/kmain.o out/kmain_startup.o out/Memory.o
	$(LD) -nostdlib -nolibc -nostartfiles -nodefaultlibs -m elf_i386 -T kernel/linker.ld $^ -o $@

out/kernel.bin: out/kernel.elf
	$(GCC_PREFIX)objcopy -Obinary $< $@

out/%.o: kernel/%.cpp kernel/linker.ld
	$(GCC) $(GCC_FLAGS) -Wall -Wextra -Iinc/ -g3 -O0 -m32 -c $< -o $@ 

out/kmain_startup.o: kernel/kmain_startup.asm
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
