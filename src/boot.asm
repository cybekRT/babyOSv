%include "global.inc"

[bits 16]
[org 0x7c00]

start:
	cli
	;mov	[floppy_id], dl

	mov	ah, 0x02
	mov	al, (KERNEL_SIZE + 511) / 512
	mov	ch, 0
	mov	cl, 2
	mov	dh, 0
	mov	dl, 0;[floppy_id]

	mov	bx, 0
	mov	es, bx
	mov	bx, KERNEL_ADDR

	int	13h
	jc 	.fail

	jmp	0x0:KERNEL_ADDR

.fail:
	mov	bx, 0xb800
	mov	es, bx
	mov	bx, 0
	mov	byte [es:bx + 0], 'F'
	mov	byte [es:bx + 2], ' '

	hlt
	jmp	.fail

floppy_id db 0

times 510 - ($ - $$) db 0
dw 0xAA55