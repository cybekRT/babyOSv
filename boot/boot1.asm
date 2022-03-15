[bits 16]
[org 0x7c00]
[cpu 386]
%include"global.inc"
%include"FAT12.inc"

xBPB:
	istruc BPB_t
		at BPB_t.jmpCode,		jmp short bootcode
						nop
		at BPB_t.oem,			db 'MSDOS5.0'
		at BPB_t.bytesPerSector,	dw 512
		at BPB_t.sectorsPerCluster,	db 1
		at BPB_t.reservedSectors,	dw 1 ;((KERNEL_SIZE + 511) / 512)
		at BPB_t.fatsCount,		db 2
		at BPB_t.rootEntries,		dw 224
		at BPB_t.totalSectors,		dw 2880
		at BPB_t.mediaType,		db 0xF0
		at BPB_t.sectorsPerFat,		dw 0
		at BPB_t.sectorsPerTrack,	dw 0
		at BPB_t.headsCount,		dw 2
		at BPB_t.hiddenSectors,		dd 0
		at BPB_t.unused,		dd 0
		at BPB_t.driveNumber,		db 0
		at BPB_t.reserved,		db 0
		at BPB_t.signature,		db 0x28
		at BPB_t.volumeId,		db '1234'
		at BPB_t.label,			db OS_NAME
		at BPB_t.systemId,		db ' babyOS '
	iend

; CHS
; 80, 2, 18

bootcode:
	mov	[xBPB + BPB_t.driveNumber], dl
	jmp	0:Init

helloMsg db OS_NAME, 0xA, 0xD, "Loading stage2..."
helloMsgLen equ ($ - helloMsg)
db helloMsgLen
;;;;;
%include"FAT12_lite.asm"
fileName db "BOOT    BIN"
FILE_SEG equ (BOOT2_ADDR >> 4)
filePtr dw FILE_SEG

Init:
	cli

	; CS set to 0x0 with jump, but DS also needs to be set to 0x0...
	mov	bx, 0
	mov	ds, bx
	mov	es, bx

	; Clear screen
	mov	ax, 3
	int	10h

	; Display hello message
	mov	ax, 0x1301
	mov	bx, 0x000F
	mov	cx, helloMsgLen
	mov	dx, 0
	mov	bp, helloMsg
	int	10h

	; Copy this sector
	mov	si, 0x7c00
	mov	di, BPB_LOC
	mov	cx, 512
	rep movsb

	call	FindFile
	call	ReadFile
	call	ExecuteBoot2
	jmp	Fail

ExecuteBoot2:
	jmp	0x0:BOOT2_ADDR

Fail:
	xchg bx, bx
	;mov	bx, 0xb800
	;mov	es, bx
	;mov	bx, 0
	;mov	byte [es:bx + 0], 'F'

	; Boot other device...
	int	0x18
	;jmp $

;;;;; Padding
	times 510 - ($ - $$) db 0x90
	dw 0xAA55
