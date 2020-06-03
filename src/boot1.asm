%include "global.inc"

[bits 16]
[org 0x7c00]

start:
	cli
	;mov	[floppy_id], dl

	mov	ax, 1
	mov	cx, (BOOT2_SIZE + 511) / 512
	mov	bx, 0
	mov	es, bx
	mov	bx, KERNEL_ADDR
xchg bx, bx

.loop:
	call	ReadSector
	inc	ax
	add	bx, 512
	loop	.loop


;	mov	ah, 0x02
;	mov	al, (BOOT2_SIZE + 511) / 512
;	mov	ch, 0
;	mov	cl, 2
	mov	dh, 0
;	mov	dl, 0;[floppy_id]
;
;	int	13h
;	jc 	.fail

	xchg bx, bx

	jmp	0x0:KERNEL_ADDR

.fail:
	mov	bx, 0xb800
	mov	es, bx
	mov	bx, 0
	mov	byte [es:bx + 0], 'F'
	mov	byte [es:bx + 2], ' '

	hlt
	jmp	.fail

%include "boot_read.asm"

floppy_id db 0

times 510 - ($ - $$) db 0
dw 0xAA55