[bits 16]
[org 0x7c00]
[cpu 386]
%include "global.inc"
%include "FAT12.inc"

; CHS
; 80, 2, 18

;;;;; Code
BPB:
	jmp short		bootcode
	nop

	oem			db 'MSDOS5.0'
	bytesPerSector		dw 512
	sectorsPerCluster	db 1
	reservedSectors		dw 1 ;((KERNEL_SIZE + 511) / 512)
	fatsCount		db 2
	rootEntries		dw 224
	totalSectors		dw 2880
	mediaType		db 0xF0
	sectorsPerFat		dw 0
	sectorsPerTrack		dw 0
	headsCount		dw 2
	hiddenSectors		dd 0
	unused			dd 0
EBR:
	driveNumber		db 0
	reserved		db 0
	signature		db 0x28
	volumeId		db '1234'
	label			db OS_NAME
	systemId		db ' babyOS '

bootcode:
	mov	[driveNumber], dl
	jmp	0:Init

;;;;; Consts
FILENAME db "BOOT    BIN"
ROOT_LOC equ (0x7c00 + 0x200)
FAT_LOC equ (ROOT_LOC + 0x200)
;;;;; Variables
rootSector db 0
rootSectorEnd db 0
rootOffset dw 0
kernelCluster dw 0
kernelPosition dw BOOT2_ADDR
fatSector dw 0xFFFF
helloMsg db OS_NAME, 0xA, 0xD, "Loading kernel..."
helloMsgLen equ ($ - helloMsg)
db helloMsgLen
;;;;;

Init:
	; CS set to 0x0 with jump, but DS also needs to be set to 0x0...
	mov	bx, 0
	mov	ds, bx

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

	; Calculate last root sector
	mov	ax, [rootEntries]
	shr	ax, 4
	mov	[rootSectorEnd], al

	; Calculate first root sector
	mov	al, [sectorsPerFat]
	mov	ah, [fatsCount]
	mul	ah
	add	ax, [reservedSectors]
	mov	[rootSector], al
	add	[rootSectorEnd], al

; Search root entry
	mov	bx, 0
	mov	ds, bx
	mov	es, bx
SearchKernel:
	cmp	word [rootOffset], 0
	jne	.dontRead
	call	ReadRoot
.dontRead:
	; Set current filename offset
	mov	si, ROOT_LOC + FAT12_DirectoryEntry.name
	add	si, [rootOffset]
	; Compare
	mov	cx, 8+3
	mov	di, FILENAME
	repe	cmpsb
	je	ReadEntry

	add	word [rootOffset], FAT12_DirectoryEntry_size
	cmp	word [rootOffset], 512
	jne	SearchKernel
	mov	word [rootOffset], 0
	jmp	SearchKernel

ReadEntry:
	mov	bx, ROOT_LOC + FAT12_DirectoryEntry.cluster
	add	bx, [rootOffset]
	push	word [bx]
	pop	word [kernelCluster]

.loop:
	cmp	word [kernelCluster], 0xFF8
	jae	ExecuteKernel
	call	ReadKernel
	call	ReadFAT
	jmp	.loop

ExecuteKernel:
	jmp	0x0:BOOT2_ADDR ; Execute kernel

Fail:
	;mov	bx, 0xb800
	;mov	es, bx
	;mov	bx, 0
	;mov	byte [es:bx+0], 'F'
	;mov	byte [es:bx+2], ' '
	;jmp	$

; Boot other device...
	int	0x18

;;;;; Functions
ReadRoot:
	mov	al, [rootSectorEnd]

	cmp	byte [rootSector], al
	je	Fail

	; LBA 2 CHS
	movzx	ax, byte [rootSector]
	inc	byte [rootSector]

	mov	bx, 0
	mov	es, bx
	mov	bx, ROOT_LOC
	jmp	ReadSector

;;;;;
; Read FAT
;;;;;
ReadKernel:
	mov	ax, word [kernelCluster]
	sub	ax, 2
	push	ax

	mov	al, byte [sectorsPerFat] ; trim high-byte
	mov	cl, [fatsCount]
	mul	cl
	push	ax

	mov	ax, [rootEntries]
	shr	ax, 4

	add	ax, [reservedSectors]
	pop	bx
	add	ax, bx
	pop	bx
	add	ax, bx

	mov	bx, 0
	mov	es, bx
	mov	bx, [kernelPosition]

	call	ReadSector
	add	word [kernelPosition], 512

	ret

;;;;;
; Read FAT
;;;;;
ReadFAT:
	mov	ax, [kernelCluster]
	shr	ax, 9
	cmp	ax, [fatSector]
	je	.dontRead

; Read FAT
	add	ax, [reservedSectors]
	push	ax
	mov	bx, 0
	mov	es, bx
	mov	bx, FAT_LOC
	call	ReadSector
; Read FAT+1
	pop	ax
	inc	ax
	mov	bx, 0
	mov	es, bx
	mov	bx, FAT_LOC + 0x200
	call	ReadSector
.dontRead:
	mov	bx, [kernelCluster]
	shr	bx, 1
	add	bx, [kernelCluster]
	and	bx, 0x1ff
	add	bx, FAT_LOC
	mov	ax, [bx]

	test	word [kernelCluster], 1
	jnz	.oddCluster
	
	and	ax, 0xfff
	mov	[kernelCluster], ax
	ret
.oddCluster:
	shr	ax, 4
	mov	[kernelCluster], ax
	ret

;;;;;
; Read sector
; ax	-	LBA
; es:bx	-	destination
;;;;;
ReadSector:
	push	bx
	; LBA 2 CHS
	mov	bl, [sectorsPerTrack]
	div	bl

	mov	cl, ah ; Sectors
	inc	cl
	xor	ah, ah

	mov	bl, [headsCount]
	div	bl

	mov	dh, ah ; Heads
	mov	ch, al ; Cylinders

	;;;;;
	mov	ah, 02h
	mov	al, 1
	mov	dl, [driveNumber]
	pop	bx

	int	13h
	jc	Fail
	ret

;;;;; Padding
times 510 - ($ - $$) db 0x90
dw 0xAA55
