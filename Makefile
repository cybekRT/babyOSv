NASM		= nasm -Isrc/
BOCHS		= bochs -f bochs.cfg

all: floppy.bin

floppy.bin: out/boot.bin out/kernel.bin
	cat $^ > $@

out/boot.bin: src/boot.asm out/kernel.bin
	$(NASM) src/boot.asm -o $@ -fbin -DKERNEL_SIZE=$(strip $(shell wc -c < out/kernel.bin))

out/kernel.bin: src/kernel.asm src/c/kernel.bin
	$(NASM) $< -fbin -o $@ -l out/kernel.lst

src/c/kernel.bin: src/c/kmain.c
	$(MAKE) -C src/c

clean:
	rm out/* floppy.bin || true
	$(MAKE) -C src/c clean

qemu: floppy.bin
	qemu-system-i386 -fda $< -boot ac -m 32 -monitor stdio

bochs: floppy.bin
	$(BOCHS) -q
