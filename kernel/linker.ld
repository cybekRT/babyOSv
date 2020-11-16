ENTRY(main)
OUTPUT_FORMAT(elf32-i386)

__stack_size = 0x100;

SECTIONS
{
	. = 0x80004000;
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

		__stack_end = .;
		. = . + __stack_size;
		__stack_beg = .;
	}

	__kernel_end = .;
}
