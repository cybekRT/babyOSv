
####################
#
#	Windows configuration
#
####################
ifeq ($(OS),Windows_NT)
	NASM				= nasm
	BOCHS				= D:/Programs/Bochs/bochsdbg-p4-smp.exe -f bochs-win.cfg
	OUT					= $(PWD)/out
	GCC_PREFIX			= i386-elf-
	QEMU				= D:/Programs/Qemu/qemu-system-i386.exe
	DD					= D:/Programs/Cygwin/bin/dd
	PCEM				= D:/Programs/PCem/PCem.exe
	VBOXMANAGE			= C:/Program\ Files/VirtualBox/VBoxManage.exe
	CFS					= ../cFS/cFS-cli/cFS-cli
	QEMU_DOS_IMG		= -hda D:/Drop/dos.img
	QEMU_DOSEXT_IMG		= 
####################
#
#	MacOS configuration
#
####################
else
	NASM				= nasm
	BOCHS				= /usr/local/osdev/bin/bochs -f bochs.cfg
	OUT					= $(PWD)/out
	GCC_PREFIX			= /usr/local/osdev/bin/i386-elf-
	QEMU				= qemu-system-i386
	DD					= dd
	PCEM				= fail
	VBOXMANAGE			= vboxmanage
	CFS					= ../cFS/cFS-cli/cFS-cli
	QEMU_DOS_IMG		= -hda /Users/cybek/dos.img
	QEMU_DOSEXT_IMG		= -hdb /Users/cybek/dos-empty.img
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
GCC_FLAGS	+= -fno-exceptions -fno-rtti
NASM_FLAGS	 = -Iboot/
QEMU_FLAGS	 = $(QEMU_DOS_IMG) $(QEMU_DOSEXT_IMG) -vga std -boot ac -m 8 -d int -monitor stdio -d int -d cpu_reset -d guest_errors

####################
#
#	Output files and dependencies
#
####################

OBJS	 = $(SOURCES:kernel/%.cpp=out/%.o)
DEPS	 = $(OBJS:%.o=%.d)
DEPS	:= $(DEPS:kernel%=out%)

####################
#
#	Default internal targets
#
####################

all: out/floppy.img

out/floppy.img: out out/boot1.bin out/boot2.bin out/kernel.bin floppy.json
	$(CFS) floppy.json >/dev/null

out:
	mkdir out $(patsubst kernel/%,out/%,$(sort $(dir $(wildcard kernel/*/*)))) 2>/dev/null || true

out/boot1.bin: boot/boot1.asm boot/FAT12.inc boot/FAT12_lite.asm
	$(NASM) $(NASM_FLAGS) $< -o $@ -l out/boot1.lst -fbin 

out/boot2.bin: boot/boot2.asm boot/FAT12.inc boot/FAT12_lite.asm
	$(NASM) $(NASM_FLAGS) $< -fbin -o $@ -l out/boot2.lst 

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
	python3 kernel/Keyboard_map.py $@ $< >/dev/null

kernel/Keyboard_map.h: kernel/Keyboard_map.inc kernel/Keyboard_map.py
	python3 kernel/Keyboard_map.py $@ $< >/dev/null

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
	rm out/*/* out/* $(AUTOGEN) 2>/dev/null || true

qemu: out/floppy.img
	$(QEMU) -fda $< $(QEMU_FLAGS) 2> /dev/null
	#-d int -no-reboot -no-shutdown

qemu-dbg: out/floppy.img
	$(QEMU) -fda $< -s -S 2> /dev/null

qemu-dos: out/floppy.img
	$(QEMU) -fda $< $(QEMU_FLAGS) -boot c 2> /dev/null
	#-d int -no-reboot -no-shutdown

bochs: out/floppy.img
	$(BOCHS) -q

pcem: out/floppy.img
	$(PCEM)

vbox: out/floppy.img
	$(VBOXMANAGE) startvm "babyOSv" -E VBOX_GUI_DBG_AUTO_SHOW=true -E VBOX_GUI_DBG_ENABLED=true

####################
#
#	PHONY Phony phony
#
####################

.PHONY: all clean qemu qemu-dbg bochs pcem vbox
