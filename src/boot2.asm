%include "global.inc"

[bits 16]
[org KERNEL_ADDR]

main16:
	; Read kernel.bin
;	mov	ah, (kmain - main16 + 511) / 512 + 1 ;0x02
;	mov	al, (KERNEL_SIZE + 511) / 512
;	mov	ch, 0
;	mov	cl, 2
;	mov	dh, 0
;	mov	dl, 0
;	mov	bx, 0
;	mov	es, bx
;	mov	bx, kmain
;	int	13h
;	jc 	.fail

	mov	ax, (kmain - main16 + 511) / 512 + 2
	mov	cx, (KERNEL_SIZE + 511) / 512
	mov	bx, 0
	mov	es, bx
	mov	bx, kmain
xchg bx, bx
.loop:
	call	ReadSector
xchg bx, bx
	inc	ax
	add	bx, 512
	loop	.loop

	mov	bx, 0xb800
	mov	es, bx
	mov	bx, 0
	inc	byte [es:bx]

	mov	bx, 0
	mov	ds, bx
	mov	es, bx

	;mov	ax, 13h
	;int	10h

	lgdt	[GDT_Handle]

	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax

	mov	bx, 0x10
	mov	ds, bx
	mov	es, bx
	mov	ss, bx

xchg bx, bx
	jmp	0x8:main32
.fail:
	mov	bx, 0xb800
	mov	es, bx
	mov	bx, 0
	mov	byte [es:bx + 0], 'F'
	mov	byte [es:bx + 2], ' '

	hlt
	jmp	.fail

%include "boot_read.asm"

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
	mov	ebx, 0xb8000
	mov	byte [ebx + 0], 'x'
	mov	byte [ebx + 2], 'y'
	mov	byte [ebx + 4], 'z'
	mov	byte [ebx + 6], ' '

	mov	eax, PageDirectory
	mov	ebx, PageEntry
	and	ebx, ~0xFFF
	or	ebx, (PAGE_DIRECTORY_FLAG_PRESENT | PAGE_DIRECTORY_FLAG_READ_WRITE | PAGE_DIRECTORY_FLAG_SUPERVISOR | PAGE_DIRECTORY_FLAG_PAGE_4K)
	mov	[eax], ebx

	mov	cr3, eax
	mov	eax, cr0
	or	eax, 0x80000001
	mov	cr0, eax

;	mov	ebx, 0xb8000
;	mov	byte [ebx + 0], 'C'
;	mov	byte [ebx + 2], 'R'
;	mov	byte [ebx + 4], '3'
;	mov	byte [ebx + 6], '.'

xchg bx, bx
	jmp	kmain

.halt:
	cli
	hlt
	jmp	.halt

times 0x1000 - ($ - $$ + KERNEL_ADDR) db 0
;times 0x1000 - ($ - $$ + KERNEL_ADDR) % 0x1000 db 0
;align 0x1000
;times 4096 - ($ - $$) % 4096 db 0

PageDirectory:
	;dd PAGE_TABLE_ENTRY(PageEntry, PAGE_DIRECTORY_FLAG_PRESENT | PAGE_DIRECTORY_FLAG_READ_WRITE | PAGE_DIRECTORY_FLAG_SUPERVISOR | PAGE_DIRECTORY_FLAG_PAGE_4K)
	;times 1023 dd 0
	times 1024 dd 0

PageEntry:
	%assign i 0
	%rep 1024
		dd PAGE_TABLE_ENTRY(i, PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR)
	%assign i i+4096
	%endrep

;times 0x3000 - ($ - $$ + KERNEL_ADDR) % 0x3000 db 0

;times (0x3000 - $) db 0

;times 8704 - ($ - $$) db 0
kmain:
;	incbin "c/kernel.bin"

times (KERNEL_ADDR + 512 - ($ - $$)) % 512 db 0
;kmain:
