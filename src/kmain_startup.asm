[bits 32]
global main
global bootloader_info_ptr
global _bootloader_info_ptr
global kernel_end
extern kmain
extern stack_beg
extern stack_end
extern __kernel_end

[section .startup]
main:
	pop	dword [bootloader_info_ptr]
	mov	esp, stack_beg

	call	kmain

.halt:
	cli
	hlt
	jmp	.halt

_bootloader_info_ptr
bootloader_info_ptr dd 0
kernel_end dd __kernel_end