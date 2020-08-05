%include "global.inc"

[bits 16]
[org 0x7c00]

start:
	cli

	mov	ax, 1
	mov	cx, (BOOT2_SIZE + 511) / 512
	mov	bx, 0
	mov	es, bx
	mov	bx, BOOT2_ADDR

.loop:
	call	ReadSector
	inc	ax
	add	bx, 512
	loop	.loop

	jmp	0x0:BOOT2_ADDR

.fail:
	mov	bx, 0xb800
	mov	es, bx
	mov	bx, 0
	mov	byte [es:bx + 0], 'F'
	mov	byte [es:bx + 2], ' '

	hlt
	jmp	.fail

%include "boot_read.asm"

times 510 - ($ - $$) db 0
dw 0xAA55