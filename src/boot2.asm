%include "global.inc"

[bits 16]
[org KERNEL_ADDR]

main16:
	mov	bx, 0
	mov	ds, bx

	; Read kernel.bin
	mov	ax, (kmain - main16 + 511) / 512 + 1
	mov	cx, (KERNEL_SIZE + 511) / 512
	mov	bx, 0
	mov	es, bx
	mov	bx, kmain
.loop:
	call	ReadSector

	inc	ax
	add	bx, 512
	loop	.loop

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
	cli
	hlt
	jmp	.legacy
	;mov	dword [MEM_MAP_entries], 1
	;mov	eax, MEM_MAP
	;mov	dword [eax + MEMMap_t.base], 0x0
	;mov	dword [eax + MEMMap_t.length], 0xfffff
	;mov	byte [eax + MEMMap_t.type], 0x1
	;jmp	.exit

%include "boot_read.asm"

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
	;mov	ebx, 0xb8000
	;mov	byte [ebx + 0], 'x'
	;mov	byte [ebx + 2], 'y'
	;mov	byte [ebx + 4], 'z'
	;mov	byte [ebx + 6], ' '

	mov	eax, PageDirectory
	mov	ebx, PageTable
	and	ebx, ~0xFFF
	or	ebx, (PAGE_DIRECTORY_FLAG_PRESENT | PAGE_DIRECTORY_FLAG_READ_WRITE | PAGE_DIRECTORY_FLAG_SUPERVISOR | PAGE_DIRECTORY_FLAG_PAGE_4K)
	mov	[eax + 4 *    0], ebx ; 0x00000000
	mov	[eax + 4 *  512], ebx ; 0x80000000
	mov	[eax + 4 * 1023], eax

	mov	cr3, eax
	mov	eax, cr0
	or	eax, 0x80000001
	mov	cr0, eax

	push	BootloaderInfo
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

times 0x1000 - ($ - $$ + KERNEL_ADDR) db 0

PageDirectory:
	;dd PAGE_TABLE_ENTRY(PageEntry, PAGE_DIRECTORY_FLAG_PRESENT | PAGE_DIRECTORY_FLAG_READ_WRITE | PAGE_DIRECTORY_FLAG_SUPERVISOR | PAGE_DIRECTORY_FLAG_PAGE_4K)
	;times 1023 dd 0
	times 1024 dd 0

PageTable:
	%assign i 0
	%rep 1024
		dd PAGE_TABLE_ENTRY(i, PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR)
	%assign i i+4096
	%endrep

times 0x3000 - KERNEL_ADDR - ($ - $$) db 0
kmain:
;	incbin "c/kernel.bin"
