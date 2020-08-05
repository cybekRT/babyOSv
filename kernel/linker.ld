ENTRY(main)
OUTPUT_FORMAT(elf32-i386)

stack_size = 0x100;

SECTIONS
{
	. = 0x80003000;
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
		*(.eh_frame)

		stack_end = .;
		. = . + stack_size;
		stack_beg = .;
	}

	__kernel_end = .;
}
