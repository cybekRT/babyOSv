%include "global.inc"
%include "FAT12.inc"

[bits 16]
[org BOOT2_ADDR]

main16:
	mov	bx, 0
	mov	ds, bx
	mov	es, bx

	mov	esp, stackBegin

	;mov	ax, 13h
	;int	10h

	; Display hello message
	mov	ax, 0x1301
	mov	bx, 0x000F
	mov	cx, helloMsg1Len
	mov	dx, 0x0200
	mov	bp, helloMsg1
	int	10h

	call	FindFile

	; Display hello message
	mov	ax, 0x1301
	mov	bx, 0x000F
	mov	cx, helloMsg2Len
	mov	dx, 0x0300
	mov	bp, helloMsg2
	int	10h

	call	ReadFile

	; Display hello message
	mov	bx, 0
	mov	es, bx
	mov	ax, 0x1301
	mov	bx, 0x000F
	mov	cx, helloMsg3Len
	mov	dx, 0x0400
	mov	bp, helloMsg3
	int	10h

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

helloMsg1 db "Searching kernel..."
helloMsg1Len equ ($ - helloMsg1)
helloMsg2 db "Reading kernel..."
helloMsg2Len equ ($ - helloMsg2)
helloMsg3 db "Starting kernel..."
helloMsg3Len equ ($ - helloMsg3)

fileName db "KERNEL  BIN"
FILE_SEG equ (KERNEL_ADDR >> 4)
filePtr dw FILE_SEG
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
	; Read CMOS ram size
	; Low byte
	mov	dx, 0x70
	mov	al, 0x30
	out dx, al
	in	al, 0x71
	mov	[.legacyTmp], al
	; High byte
	mov	dx, 0x70
	mov	al, 0x31
	out dx, al
	in	al, 0x71
	mov	[.legacyTmp+1], al
	; Add first 1024kB and change kB to B
	mov	edx, [.legacyTmp]
	add	edx, 1024
	shl	edx, 10

	; TODO: detect memory size, reserve standard BIOS and GPU ranges
	mov	dword [MEM_MAP_entries], 1
	mov	eax, MEM_MAP
	mov	dword [eax + 0], 0x0
	mov	dword [eax + 8], edx
	mov	byte [eax + 16], 0x1
	jmp	.exit
.legacyTmp dd 0

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
; Null entry
dq 0
; Ring 0 - code
istruc GDT
	at GDT.limit_0_15, dw 0xffff
	at GDT.base_0_15, dw 0
	at GDT.base_16_23, db 0
	at GDT.flags, db (GDT_FLAG_CODE_READ | GDT_FLAG_SEG_CODE | GDT_FLAG_SEG_CODE_OR_DATA | GDT_FLAG_RING_0 | GDT_FLAG_ENTRY_PRESENT)
	at GDT.limit_16_19_attributes, db 0xF | (GDT_ATTRIBUTE_32BIT_SIZE | GDT_ATTRIBUTE_GRANULARITY)
	at GDT.base_24_31, db 0
iend
; Ring 0 - data
istruc GDT
	at GDT.limit_0_15, dw 0xffff
	at GDT.base_0_15, dw 0
	at GDT.base_16_23, db 0
	at GDT.flags, db (GDT_FLAG_DATA_WRITE | GDT_FLAG_SEG_DATA | GDT_FLAG_SEG_CODE_OR_DATA | GDT_FLAG_RING_0 | GDT_FLAG_ENTRY_PRESENT)
	at GDT.limit_16_19_attributes, db 0xF | (GDT_ATTRIBUTE_32BIT_SIZE | GDT_ATTRIBUTE_GRANULARITY)
	at GDT.base_24_31, db 0
iend
; Ring 3 - code
;istruc GDT
;	at GDT.limit_0_15, dw 0xffff
;	at GDT.base_0_15, dw 0
;	at GDT.base_16_23, db 0
;	at GDT.flags, db (GDT_FLAG_CODE_READ | GDT_FLAG_SEG_CODE | GDT_FLAG_SEG_CODE_OR_DATA | GDT_FLAG_RING_3 | GDT_FLAG_ENTRY_PRESENT)
;	at GDT.limit_16_19_attributes, db 0xF | (GDT_ATTRIBUTE_32BIT_SIZE | GDT_ATTRIBUTE_GRANULARITY)
;	at GDT.base_24_31, db 0
;iend
; Ring 3 - data
;istruc GDT
;	at GDT.limit_0_15, dw 0xffff
;	at GDT.base_0_15, dw 0
;	at GDT.base_16_23, db 0
;	at GDT.flags, db (GDT_FLAG_DATA_WRITE | GDT_FLAG_SEG_DATA | GDT_FLAG_SEG_CODE_OR_DATA | GDT_FLAG_RING_3 | GDT_FLAG_ENTRY_PRESENT)
;	at GDT.limit_16_19_attributes, db 0x0 | (GDT_ATTRIBUTE_32BIT_SIZE | GDT_ATTRIBUTE_GRANULARITY)
;	at GDT.base_24_31, db 0
;iend
; TSS
;istruc GDT
;	at GDT.limit_0_15, dw 0
;	at GDT.base_0_15, dw 0
;	at GDT.base_16_23, db 0
;	at GDT.flags, db 0
;	at GDT.limit_16_19_attributes, db 0
;	at GDT.base_24_31, db 0
;iend
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
	jmp		kmain + 0x80000000
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
