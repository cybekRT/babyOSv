ENTRY(main)
OUTPUT_FORMAT(elf32-i386)

__stack_size = 0x100;

SECTIONS
{
	. = 0x80004000;
	__kernel_beg = .;
	__kernel_code_beg = .;
	.startup : {
		*(.startup)
	}
	.text : {
		*(.text)
	}
	__kernel_code_end = .;
	__kernel_data_beg = .;
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

	__kernel_data_end = .;
	__kernel_end = .;
}
