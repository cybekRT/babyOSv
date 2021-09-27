[bits 32]
	global main

	global _bootloader_info_ptr

	global _kernel_beg
	global _kernel_end
	global _kernel_code_beg
	global _kernel_code_end

	global _org_stack_beg
	global _org_stack_end
	global _org_stack_size

	extern kmain
	extern __kernel_beg
	extern __kernel_end
	extern __kernel_code_beg
	extern __kernel_code_end

	extern __stack_beg
	extern __stack_end
	extern __stack_size

	global _ctors_beg
	global _ctors_end
	extern __ctors_beg
	extern __ctors_end

[section .startup]
jmp main
db "<YOLO>"
main:
	cli

	pop	dword [_bootloader_info_ptr]
	mov	esp, __stack_beg

	mov ebx, __ctors_beg
.constructorsLoop:
	cmp	ebx, __ctors_end
	je	.afterConstructors

	call	[ebx]
	add	ebx, 4
	jmp	.constructorsLoop

.afterConstructors:
	mov	ebp, 0
	call	kmain

.halt:
	cli
	hlt
	jmp	.halt

	_bootloader_info_ptr dd 0
	_kernel_beg dd __kernel_beg
	_kernel_end dd __kernel_end
	_kernel_code_beg dd __kernel_code_beg
	_kernel_code_end dd __kernel_code_end
	_org_stack_beg dd __stack_beg
	_org_stack_end dd __stack_end
	_org_stack_size dd __stack_size
	_ctors_beg dd __ctors_beg
	_ctors_end dd __ctors_end
db "</YOLO>"