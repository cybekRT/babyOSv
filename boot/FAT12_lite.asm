fatLBA dw 0
rootLBA dw 0
dataLBA dw 0

rootEntryOffset dw 0
fileCluster dw 0
fatLBARead dw 0xFFFF

FindFile:
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
	mov	si, fileName

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
	mov	[fileCluster], ax
	ret

ReadFile:
.loop:
	cmp	word [cs:fileCluster], CLUSTER_LAST
	je	.exit

	mov	si, [cs:fileCluster]
	shr	si, 1
	add	si, [cs:fileCluster]

	mov	ax, si
	shr	ax, 9
	cmp	ax, [cs:fatLBARead]
	je	.noRead

	mov	[cs:fatLBARead], ax
	add	ax, [cs:fatLBA]

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
	mov	ax, [cs:dataLBA]
	add	ax, [cs:fileCluster]
	sub	ax, 2

	mov	bx, [cs:filePtr]
	mov	es, bx
	mov	bx, 0
	call	ReadSector

	mov	bx, 0
	mov	es, bx

	add	word [cs:filePtr], 0x20;0
	cmp word [cs:filePtr], FILE_SEG
	jb	FailX

	; Calculate next cluster
	mov	bx, [cs:fileCluster]
	shr	bx, 1
	add	bx, [cs:fileCluster]
	add	bx, FAT_LOC

	test	word [cs:fileCluster], 1
	jnz	.odd

.even:
	mov	ax, [cs:bx]
	and	ax, 0x0FFF
	mov	[cs:fileCluster], ax
	jmp	.loop

.odd:
	mov	ax, [cs:bx]
	shr	ax, 4
	mov	[cs:fileCluster], ax
	jmp	.loop

.exit:
	ret

FailX:
	xchg bx, bx

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
	mov	bl, [cs:BPB_LOC + BPB_t.sectorsPerTrack]
	div	bl

	mov	cl, ah ; Sectors
	inc	cl
	xor	ah, ah

	mov	bl, [cs:BPB_LOC + BPB_t.headsCount]
	div	bl

	mov	dh, ah ; Heads
	mov	ch, al ; Cylinders

	; perform read
	mov	ah, 02h
	mov	al, 1
	mov	dl, [cs:BPB_LOC + BPB_t.driveNumber]
	pop	bx

	xchg bx, bx
	int	13h
	jc	Fail
	ret
