ENTRY(main)
OUTPUT_FORMAT(binary)

SECTIONS
{
	. = 0x00002100;
	.text : {
		*(.text)
	}
	.rodata ALIGN (0x1000) : {
		*(.rodata)
	}
	.data ALIGN (0x1000) : {
		*(.data)
	}
	.bss : {
		sbss = .;
		*(COMMON)
		*(.bss)
		ebss = .;
	}
}