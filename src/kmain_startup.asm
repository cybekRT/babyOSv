[bits 32]
extern kmain
global main

[section .startup]
main:
	call	kmain

.halt:
	cli
	hlt
	jmp	.halt
