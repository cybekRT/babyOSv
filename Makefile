
NASM		= nasm -Iboot/

ifeq ($(OS),Windows_NT)
	BOCHS		= D:\Programs\Bochs\bochsdbg-p4-smp.exe -f bochs-win.cfg
	OUT		= $(PWD)/out
	GCC_PREFIX	= i386-elf-
	QEMU		= D:\Programs\Qemu\qemu-system-i386.exe
	DD		= D:\Programs\Cygwin\bin\dd
	# qemu-system-i386
	PCEM		= D:\Programs\PCem\PCem.exe
	VBOXMANAGE	= C:/Program\ Files/VirtualBox/VBoxManage.exe
else
	BOCHS		= bochs -f bochs.cfg
	OUT		= $(PWD)/out
	GCC_PREFIX	= /usr/local/osdev/bin/i386-elf-
	QEMU		= qemu-system-i386
	DD		= dd
	PCEM		= 
	VBOXMANAGE	= vboxmanage
endif

GCC			= $(GCC_PREFIX)gcc
LD			= $(GCC_PREFIX)ld
GCC_FLAGS		= -include kernel/global.h -std=gnu++1z -mgeneral-regs-only -fno-isolate-erroneous-paths-attribute -fno-asynchronous-unwind-tables

SOURCES = $(shell find kernel -name *.cpp)

OBJS  = $(SOURCES:kernel/%.cpp=out/%.o)
DEPS  = $(SOURCES:%.cpp=%.d)
DEPS := $(DEPS:kernel%=out%)

#x:
#	echo $(SOURCES)
#	echo $(DEPS)

all: floppy.img

floppy.img: out/boot1.bin out/boot2.bin out/kernel.bin
	cat $^ out/boot1.bin > $@
	$(DD) if=/dev/zero of=$@ bs=1 count=0 seek=1474560

out/boot1.bin: boot/boot1.asm out/boot2.bin
	$(NASM) $< -o $@ -l out/boot1.lst -fbin -DBOOT2_SIZE=$(strip $(shell wc -c < out/boot2.bin))

out/boot2.bin: boot/boot2.asm out/kernel.bin
	$(NASM) $< -fbin -o $@ -l out/boot2.lst -DKERNEL_SIZE=$(strip $(shell wc -c < out/kernel.bin))

out/kernel.elf: out/kmain_startup.o $(OBJS)
	$(LD) -nostdlib -nolibc -nostartfiles -nodefaultlibs -m elf_i386 -T kernel/linker.ld $^ -o $@

out/kernel.bin: out/kernel.elf
	$(GCC_PREFIX)objcopy -Obinary $< $@

out/%.o: kernel/%.cpp kernel/linker.ld
	$(GCC) $(GCC_FLAGS) -Wall -Wextra -Iinc/ -g3 -O0 -m32 -c $< -o $@ 

out/kmain_startup.o: kernel/kmain_startup.asm
	$(NASM) -felf $< -o $@

out/%.d: kernel/%.cpp
	$(GCC) $(GCC_FLAGS) -c -MD -MF $@ $<

-include $(DEPS)

clean:
	rm out/* floppy.img || true
#	$(MAKE) -C src clean

qemu: floppy.img
	$(QEMU) -fda $< -boot ac -m 32 -d int -monitor stdio -d int -no-reboot -no-shutdown 
	# -no-reboot -no-shutdown 

bochs: floppy.img
	$(BOCHS) -q

pcem: floppy.img
	$(PCEM)

vbox: floppy.img
	$(VBOXMANAGE) startvm "babyOSv" -E VBOX_GUI_DBG_AUTO_SHOW=true -E VBOX_GUI_DBG_ENABLED=true
