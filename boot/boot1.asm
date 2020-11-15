[bits 16]
[org 0x7c00]
[cpu 386]
%include "global.inc"
;%include "FAT12.inc"

struc BPB_t
	.jmpCode		resb 3
	.oem			resb 8
	.bytesPerSector		resw 1
	.sectorsPerCluster	resb 1
	.reservedSectors	resw 1
	.fatsCount		resb 1
	.rootEntries		resw 1
	.totalSectors		resw 1
	.mediaType		resb 1
	.sectorsPerFat		resw 1
	.sectorsPerTrack	resw 1
	.headsCount		resw 1
	.hiddenSectors		resd 1
	.unused			resd 1
	.driveNumber		resb 1
	.reserved		resb 1
	.signature		resb 1
	.volumeId		resb 4
	.label			resb 11
	.systemId		resb 8
endstruc

struc FAT12_DirectoryEntry
	;.name			resb 8+3
	.name			resb 8
	.ext			resb 3
	.attributes		resb 1
	.reserved		resb 2
	.createTime		resb 2
	.createDate		resb 2
	.accessDate		resb 2
	.clusterHigh		resb 2
	.modificationTime	resb 2
	.modificationDate	resb 2
	.cluster		resb 2
	.size			resb 4
endstruc

CLUSTER_LAST	equ 0xFF8

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

helloMsg db OS_NAME, 0xA, 0xD, "Loading kernel..."
helloMsgLen equ ($ - helloMsg)
db helloMsgLen
;;;;;

fatLBA dw 0
rootLBA dw 0
dataLBA dw 0
kernelName db "BOOT    BIN"

rootEntryOffset dw 0
kernelCluster dw 0
kernelDstSector dw BOOT2_ADDR
fatLBARead dw 0

Init:
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

	call	FindKernel
	call	ReadKernel
	call	ExecuteKernel
	jmp	Fail

FindKernel:
	xchg bx, bx
	; Calculate FAT LBA
	mov	ax, [BPB_LOC + BPB_t.reservedSectors]
	add	ax, [BPB_LOC + BPB_t.hiddenSectors]
	mov	[fatLBA], ax

	; Calculate root LBA
	mov	ax, [BPB_LOC + BPB_t.sectorsPerFat]
	movzx	bx, byte [BPB_LOC + BPB_t.fatsCount]
	mul	bx
	add	ax, [fatLBA]
	mov	[rootLBA], ax

	; Calculate data LBA
	mov	dx, 0
	mov	ax, [BPB_LOC + BPB_t.rootEntries]
	mov	bx, 16 ; 512 / 32
	div	bx
	add	ax, [rootLBA]
	mov	[dataLBA], ax

.readRoot:
	; Read root
	mov	ax, [rootLBA]
	mov	bx, 0
	mov	es, bx
	mov	bx, ROOT_LOC
	call	ReadSector

	; Find file
	mov	di, ROOT_LOC
	add	di, [rootEntryOffset]
.loop:
	mov	cx, 8+3
	mov	si, kernelName

	rep cmpsb
	je	.found

	add	di, cx
	add	di, FAT12_DirectoryEntry_size - 8 - 3

	cmp	di, ROOT_LOC + 512
	jb	.loop

	inc	word [rootLBA]
	jmp	.readRoot

.found:
	sub	di, 8+3
	mov	ax, [di + FAT12_DirectoryEntry.cluster]
	mov	[kernelCluster], ax

ReadKernel:
.loop:
	cmp	word [kernelCluster], CLUSTER_LAST
	je	.exit

	mov	si, [kernelCluster]
	shr	si, 1
	add	si, [kernelCluster]

	mov	ax, si
	shr	ax, 9
	cmp	ax, [fatLBARead]
	je	.noRead

	; Read FAT sector
	push	ax
	mov	bx, 0
	mov	es, bx
	mov	bx, FAT_LOC
	call	ReadSector

	; Read FAT+1 sector
	pop	ax
	inc	ax
	mov	bx, 0
	mov	es, bx
	mov	bx, FAT2_LOC
	call	ReadSector

.noRead:
	mov	ax, [dataLBA]
	add	ax, [kernelCluster]

	mov	bx, 0
	mov	es, bx
	mov	bx, [kernelDstSector]
	xchg bx, bx
	call	ReadSector

	add	word [kernelDstSector], 512

	; Calculate next cluster
	mov	bx, [kernelCluster]
	shr	bx, 1
	add	bx, [kernelCluster]
	add	bx, FAT_LOC

	test	word [kernelCluster], 1
	jnz	.odd

.even:
	mov	ax, [bx]
	and	ax, 0x0FFF
	mov	[kernelCluster], ax
	jmp	.loop

.odd:
	mov	ax, [bx]
	shr	ax, 4
	mov	[kernelCluster], ax
	jmp	.loop

.exit:
	ret

ExecuteKernel:
	jmp	0x0:BOOT2_ADDR

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Read sector
; ax	-	LBA
; es:bx	-	destination
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ReadSector:
	push	bx
	; LBA 2 CHS
	mov	bl, [xBPB + BPB_t.sectorsPerTrack]
	div	bl

	mov	cl, ah ; Sectors
	inc	cl
	xor	ah, ah

	mov	bl, [xBPB + BPB_t.headsCount]
	div	bl

	mov	dh, ah ; Heads
	mov	ch, al ; Cylinders

	; perform read
	mov	ah, 02h
	mov	al, 1
	mov	dl, [xBPB + BPB_t.driveNumber]
	pop	bx

	int	13h
	jc	Fail
	ret

Fail:
	mov	bx, 0xb800
	mov	es, bx
	mov	bx, 0
	mov	byte [es:bx + 0], 'F'
	mov	byte [es:bx + 2], ' '

	; Boot other device...
	cli
	hlt
	int	0x18

;;;;; Padding
	times 510 - ($ - $$) db 0x90
	dw 0xAA55
