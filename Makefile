
####################
#
#	Windows configuration
#
####################
ifeq ($(OS),Windows_NT)
	NASM		= nasm
	BOCHS		= D:/Programs/Bochs/bochsdbg-p4-smp.exe -f bochs-win.cfg
	OUT			= $(PWD)/out
	GCC_PREFIX	= i386-elf-
	QEMU		= D:/Programs/Qemu/qemu-system-i386.exe
	DD			= D:/Programs/Cygwin/bin/dd
	PCEM		= D:/Programs/PCem/PCem.exe
	VBOXMANAGE	= C:/Program\ Files/VirtualBox/VBoxManage.exe
####################
#
#	MacOS configuration
#
####################
else
	NASM		= nasm
	BOCHS		= /usr/local/osdev/bin/bochs -f bochs.cfg
	OUT			= $(PWD)/out
	GCC_PREFIX	= /usr/local/osdev/bin/i386-elf-
	QEMU		= qemu-system-i386
	DD			= dd
	PCEM		= echo "Everyone loves Catalina..."
	VBOXMANAGE	= vboxmanage
endif

####################
#
#	Internal configuration
#
####################

SOURCES	= $(shell find kernel -name *.cpp) kernel/Keyboard_map.cpp
AUTOGEN	= kernel/Keyboard_map.cpp kernel/Keyboard_map.h

####################
#
#	Flags
#
####################

GCC			 = $(GCC_PREFIX)gcc
LD			 = $(GCC_PREFIX)ld
GCC_FLAGS	 = -include kernel/global.h -Ikernel/ -Iout/
GCC_FLAGS	+= -mgeneral-regs-only -fno-isolate-erroneous-paths-attribute -fno-asynchronous-unwind-tables
GCC_FLAGS	+= -Wall -Wextra -g3 -O0 -m32 -std=gnu++1z
NASM_FLAGS	 = -Iboot/

####################
#
#	Output files and dependencies
#
####################

OBJS	 = $(SOURCES:kernel/%.cpp=out/%.o) 
DEPS	 = $(SOURCES:%.cpp=%.d)
DEPS	:= $(DEPS:kernel%=out%)

####################
#
#	Default internal targets
#
####################

all: floppy.img

floppy.img: out/boot1.bin out/boot2.bin out/kernel.bin
	cat $^ out/boot1.bin > $@
	$(DD) if=/dev/zero of=$@ bs=1 count=0 seek=1474560

out/boot1.bin: boot/boot1.asm out/boot2.bin
	$(NASM) $(NASM_FLAGS) $< -o $@ -l out/boot1.lst -fbin -DBOOT2_SIZE=$(strip $(shell wc -c < out/boot2.bin))

out/boot2.bin: boot/boot2.asm out/kernel.bin
	$(NASM) $(NASM_FLAGS) $< -fbin -o $@ -l out/boot2.lst -DKERNEL_SIZE=$(strip $(shell wc -c < out/kernel.bin))

out/kernel.elf: out/kmain_startup.o $(OBJS)
	$(LD) -nostdlib -nolibc -nostartfiles -nodefaultlibs -m elf_i386 -T kernel/linker.ld $^ -o $@

out/kernel.bin: out/kernel.elf
	$(GCC_PREFIX)objcopy -Obinary $< $@

out/%.o: kernel/%.cpp kernel/linker.ld
	$(GCC) $(GCC_FLAGS) -c $< -o $@ 

out/%.o: out/%.cpp kernel/linker.ld
	$(GCC) $(GCC_FLAGS) -c $< -o $@ 

out/kmain_startup.o: kernel/kmain_startup.asm
	$(NASM) $(NASM_FLAGS) -felf $< -o $@

out/%.d: kernel/%.cpp
	$(GCC) $(GCC_FLAGS) -MM -MT $(@:%.d=%.o) -MF $@ $<

####################
#
#	Autogen
#
####################

out/Keyboard_map.o: kernel/Keyboard_map.cpp kernel/Keyboard_map.h

kernel/Keyboard_map.cpp: kernel/Keyboard_map.inc kernel/Keyboard_map.h kernel/Keyboard_map.py
	python3 kernel/Keyboard_map.py $@ $<

kernel/Keyboard_map.h: kernel/Keyboard_map.inc kernel/Keyboard_map.py
	python3 kernel/Keyboard_map.py $@ $<

####################
#
#	Dependencies
#
####################

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif

####################
#
#	Other targets
#
####################

clean:
	rm out/* $(AUTOGEN) floppy.img 2>/dev/null || true

qemu: floppy.img
	$(QEMU) -fda $< -boot ac -m 32 -d int -monitor stdio 2> /dev/null
	#-d int -no-reboot -no-shutdown 

qemu-dbg: floppy.img
	$(QEMU) -fda $< -boot ac -m 32 -d int -s -S -monitor stdio 2> /dev/null

bochs: floppy.img
	$(BOCHS) -q

pcem: floppy.img
	$(PCEM)

vbox: floppy.img
	$(VBOXMANAGE) startvm "babyOSv" -E VBOX_GUI_DBG_AUTO_SHOW=true -E VBOX_GUI_DBG_ENABLED=true

####################
#
#	PHONY Phony phony
#
####################

.PHONY: all clean qemu qemu-dbg bochs pcem vbox
