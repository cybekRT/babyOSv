%include "global.inc"
%include "FAT12.inc"

[bits 16]
[org BOOT2_ADDR]

main16:
	mov	bx, 0
	mov	ds, bx

	mov	esp, stackBegin

	mov	ax, 13h
	;int	10h

	call	FindKernel
	call	ReadKernel

	mov	bx, 0xb800
	mov	es, bx
	mov	bx, 0
	inc	byte [es:bx]

	call	Memory_Init

	lgdt	[GDT_Handle]

	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax

	mov	bx, 0x10
	mov	ds, bx
	mov	es, bx
	mov	ss, bx

	jmp	0x8:main32

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include"FAT12_lite.asm"

kernelName db "KERNEL  BIN"
kernelDstSector dw KERNEL_ADDR
xBPB equ BPB_LOC

Fail:
	mov	bx, 0xb800
	mov	es, bx
	mov	bx, 0

	mov	byte [es:bx+0], 'F'
	mov	byte [es:bx+2], 'a'
	mov	byte [es:bx+4], 'i'
	mov	byte [es:bx+6], 'l'
	mov	byte [es:bx+8], '!'

	cli
	hlt
	jmp	Fail


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MEM_MAP_entries_max equ 8
MEM_MAP_entries dd 0
MEM_MAP times MEM_MAP_entries_max * 24 db 0
Memory_Init:
	xor	ebx, ebx
	mov	edx, 0x534d4150

	mov	di, 0
	mov	es, di
	mov	di, MEM_MAP
.loop:
	mov	ecx, 24
	mov	eax, 0xe820
	int	0x15

	jc	.legacy
	test	ebx, ebx
	jz	.exit

	add	di, 24
	inc	dword [MEM_MAP_entries]

	cmp	dword [MEM_MAP_entries], MEM_MAP_entries_max
	ja	.fail

	jmp	.loop

.exit:
	ret
.fail:
	cli
	hlt
	jmp	.fail
.legacy:
	; TODO: detect memory size, reserve standard BIOS and GPU ranges
	mov	dword [MEM_MAP_entries], 1
	mov	eax, MEM_MAP
	mov	dword [eax + 0], 0x0
	mov	dword [eax + 8], 0xfffff
	mov	byte [eax + 16], 0x1
	jmp	.exit

;%include "boot_read.asm"

%include "bootloader_info.inc"
%include "GDT.inc"
%include "Paging.inc"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Global Descriptor Table
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
GDT_data:
dq 0
istruc GDT
	at GDT.limit_0_15, dw 0xffff
	at GDT.base_0_15, dw 0
	at GDT.base_16_23, db 0
	at GDT.flags, db (GDT_FLAG_CODE_READ | GDT_FLAG_SEG_CODE | GDT_FLAG_SEG_CODE_OR_DATA | GDT_FLAG_RING_0 | GDT_FLAG_ENTRY_PRESENT)
	at GDT.limit_16_19_attributes, db 0xF | (GDT_ATTRIBUTE_32BIT_SIZE | GDT_ATTRIBUTE_GRANULARITY)
	at GDT.base_24_31, db 0
iend

istruc GDT
	at GDT.limit_0_15, dw 0xffff
	at GDT.base_0_15, dw 0
	at GDT.base_16_23, db 0
	at GDT.flags, db (GDT_FLAG_DATA_WRITE | GDT_FLAG_SEG_DATA | GDT_FLAG_SEG_CODE_OR_DATA | GDT_FLAG_RING_0 | GDT_FLAG_ENTRY_PRESENT)
	at GDT.limit_16_19_attributes, db 0xF | (GDT_ATTRIBUTE_32BIT_SIZE | GDT_ATTRIBUTE_GRANULARITY)
	at GDT.base_24_31, db 0
iend
GDT_data_end:

GDT_Handle:
	dw (GDT_data_end - GDT_data)
	dd GDT_data

[bits 32]
main32:
	mov	eax, PageDirectory
	mov	ebx, PageTable
	and	ebx, ~0xFFF
	or	ebx, (PAGE_DIRECTORY_FLAG_PRESENT | PAGE_DIRECTORY_FLAG_READ_WRITE | PAGE_DIRECTORY_FLAG_SUPERVISOR | PAGE_DIRECTORY_FLAG_PAGE_4K)
	mov	[eax + 4 *    0], ebx ; 0x00000000
	mov	[eax + 4 *  512], ebx ; 0x80000000
	
	; Recursive mapping
	mov	ebx, PageDirectory
	and	ebx, ~0xFFF
	or	ebx, (PAGE_DIRECTORY_FLAG_PRESENT | PAGE_DIRECTORY_FLAG_READ_WRITE | PAGE_DIRECTORY_FLAG_SUPERVISOR | PAGE_DIRECTORY_FLAG_PAGE_4K)
	mov	[eax + 4 * 1023], ebx

	mov	cr3, eax
	mov	eax, cr0
	or	eax, 0x80000001
	mov	cr0, eax

	; Update GDT
	; TODO: check if this can be done simultaneously with paging
	or	dword [GDT_Handle + 2], 0x80000000
	lgdt	[GDT_Handle + 0x80000000]

	push	BootloaderInfo
	xchg bx, bx
	jmp	kmain + 0x80000000
.halt:
	cli
	hlt
	jmp	.halt

BootloaderInfo:
istruc bootloader_info
	at bootloader_info.memoryEntriesCount, dd MEM_MAP_entries
	at bootloader_info.memoryEntries, dd MEM_MAP
	at bootloader_info.pageDirectory, dd PageDirectory
iend

stackEnd:
times 128 db 0
stackBegin:

times 0x2000 - ($ - $$ + BOOT2_ADDR) db 0

PageDirectory:
	times 1024 dd 0

PageTable:
	%assign i 0
	%rep 1024
		dd PAGE_TABLE_ENTRY(i, PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR)
	%assign i i+4096
	%endrep

times KERNEL_ADDR - BOOT2_ADDR - ($ - $$) db 0
kmain:
;	incbin "c/kernel.bin"

;%if (KERNEL_ADDR + KERNEL_SIZE) >= 0x7c00
;	%error "Kernel too big!"
;%endif
