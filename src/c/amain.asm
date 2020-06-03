[bits 32]
extern _kmain
global main

main:
	call	_kmain

.halt:
	cli
	hlt
	jmp	.halt
