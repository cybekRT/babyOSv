;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Read sector
; ax	-	LBA
; es:bx	-	destination
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ReadSector:
	push	ax
	push	cx
	push	dx
	push	bx
	; LBA 2 CHS
	mov	bl, 18;[sectorsPerTrack]
	div	bl

	mov	cl, ah ; Sectors
	inc	cl
	xor	ah, ah

	mov	bl, 2;[headsCount]
	div	bl

	mov	dh, ah ; Heads
	mov	ch, al ; Cylinders

	;;;;;
	mov	ah, 02h
	mov	al, 1
	mov	dl, 0;[driveNumber]
	pop	bx

	int	13h
;xchg bx, bx
	jc	.fail
	pop	dx
	pop	cx
	pop	ax
	ret
.fail:
	mov	bx, 0xb800
	mov	es, bx
	mov	bx, 0
	mov	byte [es:bx + 0], 'F'
	mov	byte [es:bx + 2], ' '

	hlt
	jmp	.fail
