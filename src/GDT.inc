GDT_FLAG_CODE_READ		equ (1 << 1)
GDT_FLAG_DATA_WRITE		equ (1 << 1)
GDT_FLAG_CONFORMING		equ (1 << 2)
GDT_FLAG_SEG_DATA		equ (0 << 3)
GDT_FLAG_SEG_CODE		equ (1 << 3) ; executable bit
GDT_FLAG_SEG_CODE_OR_DATA	equ (1 << 4)

GDT_FLAG_RING_0			equ (0b00 << 5)
GDT_FLAG_RING_1			equ (0b01 << 5)
GDT_FLAG_RING_2			equ (0b10 << 5)
GDT_FLAG_RING_3			equ (0b11 << 5)

GDT_FLAG_ENTRY_PRESENT		equ (1 << 7)

GDT_ATTRIBUTE_UNUSED		equ (1 << 4)
GDT_ATTRIBUTE_32BIT_SIZE	equ (1 << 6)
GDT_ATTRIBUTE_GRANULARITY	equ (1 << 7)

struc GDT
	.limit_0_15 resw 1,
	.base_0_15 resw 1,
	.base_16_23 resb 1,
	.flags resb 1,
	.limit_16_19_attributes resb 1,
	.base_24_31 resb 1
endstruc
