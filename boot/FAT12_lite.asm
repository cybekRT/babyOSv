%include"FAT12.inc"

FILENAME	db "BOOT    BIN"
BPB_LOC		equ 0x500
ROOT_LOC	equ 0x700
FAT_LOC		equ 0x900

rootSector db 0
rootSectorEnd db 0
rootOffset dw 0
kernelCluster dw 0
kernelPosition dw BOOT2_ADDR
fatSector dw 0xFFFF

LoadKernel_Init:
	mov	ax, [BPB_LOC + FAT12_BPB.rootEntriesCount]
	shr	ax, 4
	mov	[rootSectorEnd], al

	; Calculate first root sector
	mov	al, [BPB_LOC + FAT12_BPB.sectorsPerFat]
	mov	ah, [BPB_LOC + FAT12_BPB.fatsCount]
	mul	ah
	add	ax, [BPB_LOC + FAT12_BPB.reservedSectors]
	mov	[rootSector], al
	add	[rootSectorEnd], al

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
; Read kernel
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
; Read root
;;;;;
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
