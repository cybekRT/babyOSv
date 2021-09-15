
####################
#
#	Windows configuration
#
####################
ifeq ($(OS),Windows_NT)
	NASM				:= nasm
	BOCHS				:= D:/Programs/Bochs/bochsdbg-p4-smp.exe -f bochs-win.cfg
	OUT					:= $(PWD)/out
	GCC_PREFIX			:= i386-elf-
	QEMU				:= D:/Programs/Qemu/qemu-system-i386.exe
	DD					:= D:/Programs/Cygwin/bin/dd
	PCEM				:= D:/Programs/PCem/PCem.exe
	VBOXMANAGE			:= C:/Program\ Files/VirtualBox/VBoxManage.exe
	CFS					:= ../cFS/cFS-cli/cFS-cli
	QEMU_DOS_IMG		:= -hda D:/Drop/dos.img
	QEMU_DOSEXT_IMG		:= 
####################
#
#	MacOS configuration
#
####################
else
	NASM				:= nasm
	BOCHS				:= bochs -f bochs.cfg
	OUT					:= $(PWD)/out
	GCC_PREFIX			:= /usr/local/osdev/bin/i386-elf-
	QEMU				:= qemu-system-i386
	DD					:= dd
	PCEM				:= fail
	VBOXMANAGE			:= vboxmanage
	CFS					:= ../cFS/cFS-cli/cFS-cli
	QEMU_DOS_IMG		:= -hda /Users/cybek/dos.img
	QEMU_DOSEXT_IMG		:= -hdb /Users/cybek/dos-empty.img
endif

ifeq ($(CHROMEOS),1)
GCC_PREFIX=i686-linux-gnu-
endif

####################
#
#	Internal configuration
#
####################

# If there's no "*.cpp", 'shell find' executes windows version of find...
AUTOGEN_OUT	:= out/gen
AUTOGEN		:= $(AUTOGEN_OUT)/Keyboard_map.cpp $(AUTOGEN_OUT)/Keyboard_map.hpp
SOURCES		:= $(shell sh -c "find kernel -name *.cpp")
OUT_DIRS	:= $(shell sh -c "find kernel -type d")
OUT_DIRS	:= $(OUT_DIRS:kernel%=out%) $(AUTOGEN_OUT)

####################
#
#	Flags
#
####################

GCC			:= $(GCC_PREFIX)gcc
LD			:= $(GCC_PREFIX)ld
GCC_FLAGS	:= -include kernel/global.h -Ikernel/ -Ikernel/Core -I$(AUTOGEN_OUT)/
GCC_FLAGS	+= -Wall -Wextra -Wno-unused-parameter -g3 -O0 -m32 -march=i486 -std=gnu++1z
GCC_FLAGS	+= -mgeneral-regs-only -fno-isolate-erroneous-paths-attribute -fno-asynchronous-unwind-tables
GCC_FLAGS	+= -fno-exceptions -fno-rtti -fno-omit-frame-pointer -fno-use-cxa-atexit -fno-stack-protector
NASM_FLAGS	:= -Iboot/
QEMU_FLAGS	:= $(QEMU_DOS_IMG) $(QEMU_DOSEXT_IMG) -vga std -boot ac -m 8 -d int -monitor stdio -d int -d cpu_reset -d guest_errors

####################
#
#	Output files and dependencies
#
####################

OBJS	:= $(SOURCES:kernel/%.cpp=out/%.o)
DEPS	:= $(OBJS:%.o=%.d)
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
	mkdir $(OUT_DIRS) 2>/dev/null || true

out/boot1.bin: boot/boot1.asm boot/FAT12.inc boot/FAT12_lite.asm
	$(NASM) $(NASM_FLAGS) $< -o $@ -l out/boot1.lst -fbin

out/boot2.bin: boot/boot2.asm boot/FAT12.inc boot/FAT12_lite.asm
	$(NASM) $(NASM_FLAGS) $< -fbin -o $@ -l out/boot2.lst

out/kernel.elf: out/kmain_startup.o $(OBJS)
	$(LD) -nostdlib -nolibc -nostartfiles -nodefaultlibs -m elf_i386 -T kernel/linker.ld $^ -o $@

out/kernel.bin: out/kernel.elf
	$(GCC_PREFIX)objcopy -Obinary $< $@

out/%.o: kernel/%.cpp $(AUTOGEN) kernel/linker.ld
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

$(AUTOGEN_OUT)/Keyboard_map.cpp: kernel/Input/Keyboard_map.inc $(AUTOGEN_OUT)/Keyboard_map.hpp kernel/Input/Keyboard_map.py
	python3 kernel/Input/Keyboard_map.py $@ $< >/dev/null

$(AUTOGEN_OUT)/Keyboard_map.hpp: kernel/Input/Keyboard_map.inc kernel/Input/Keyboard_map.py
	python3 kernel/Input/Keyboard_map.py $@ $< >/dev/null

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
