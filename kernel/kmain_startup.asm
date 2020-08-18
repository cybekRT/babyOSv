[bits 32]
global main
global bootloader_info_ptr
global kernel_end
global org_stack_beg
global org_stack_end
global org_stack_size

extern kmain
extern __stack_beg
extern __stack_end
extern __stack_size
extern __kernel_end

[section .startup]
main:
	cli

	pop	dword [bootloader_info_ptr]
	mov	esp, __stack_beg

	call	kmain

.halt:
	cli
	hlt
	jmp	.halt

bootloader_info_ptr dd 0
kernel_end dd __kernel_end
org_stack_beg dd __stack_beg
org_stack_end dd __stack_end
org_stack_size dd __stack_size