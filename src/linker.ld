ENTRY(main)
/*OUTPUT_FORMAT(binary)*/
OUTPUT_FORMAT(binary)

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
	}
}
