ENTRY(main)
OUTPUT_FORMAT(binary)

stack_size = 0x400;

SECTIONS
{
	. = 0x00003000;
	.startup : {
		*(.startup)
	}
	.text : {
		*(.text)
	}
	.data : {
		*(.data)
		*(.rodata)
		*(.cstring)
		*(COMMON)
		*(.bss)
		
		stack_end = .;
		. = . + stack_size;
		stack_beg = .;
	}

	_kernel_end = .;
}
