[bits 32]
global main
global bootloader_info_ptr
global _bootloader_info_ptr
global _kernelx
extern _kmain
extern stack_beg
extern stack_end
extern _kernel_end

[section .startup]
main:
	pop	dword [bootloader_info_ptr]
	mov	esp, stack_beg

	call	_kmain

.halt:
	cli
	hlt
	jmp	.halt

_bootloader_info_ptr
bootloader_info_ptr dd 0
_kernelx dd _kernel_end