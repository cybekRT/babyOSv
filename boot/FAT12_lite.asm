%include"FAT12.inc"

fatLBA dw 0
rootLBA dw 0
dataLBA dw 0

rootEntryOffset dw 0
kernelCluster dw 0
fatLBARead dw 0xFFFF

FindKernel:
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

	mov	[fatLBARead], ax
	add	ax, [fatLBA]

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
	sub	ax, 2

	mov	bx, 0
	mov	es, bx
	mov	bx, [kernelDstSector]
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
	mov	bl, [BPB_LOC + BPB_t.sectorsPerTrack]
	div	bl

	mov	cl, ah ; Sectors
	inc	cl
	xor	ah, ah

	mov	bl, [BPB_LOC + BPB_t.headsCount]
	div	bl

	mov	dh, ah ; Heads
	mov	ch, al ; Cylinders

	; perform read
	mov	ah, 02h
	mov	al, 1
	mov	dl, [BPB_LOC + BPB_t.driveNumber]
	pop	bx

	int	13h
	jc	Fail
	ret
