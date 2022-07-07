namespace Memory
{
	namespace GDT
	{
		// GDT_FLAG_CODE_READ		equ (1 << 1)
		// GDT_FLAG_DATA_WRITE		equ (1 << 1)
		// GDT_FLAG_CONFORMING		equ (1 << 2)
		// GDT_FLAG_SEG_DATA		equ (0 << 3)
		// GDT_FLAG_SEG_CODE		equ (1 << 3) ; executable bit
		// GDT_FLAG_SEG_CODE_OR_DATA	equ (1 << 4)

		// GDT_FLAG_RING_0			equ (0b00 << 5)
		// GDT_FLAG_RING_1			equ (0b01 << 5)
		// GDT_FLAG_RING_2			equ (0b10 << 5)
		// GDT_FLAG_RING_3			equ (0b11 << 5)

		// GDT_FLAG_ENTRY_PRESENT		equ (1 << 7)

		// GDT_ATTRIBUTE_UNUSED		equ (1 << 4)
		// GDT_ATTRIBUTE_32BIT_SIZE	equ (1 << 6)
		// GDT_ATTRIBUTE_GRANULARITY	equ (1 << 7)

		struct GDTEntry_Access
		{
			/**
			 * P: Present bit. Allows an entry to refer to a valid segment. Must be set (1) for any valid segment.
			 */
			u8 present : 1;

			/**
			 * DPL: Descriptor privilege level field. Contains the CPU Privilege level of the segment. 0 = highest privilege (kernel), 3 = lowest privilege (user applications).
			 */
			u8 ring : 2;

			/**
			 * S: Descriptor type bit. If clear (0) the descriptor defines a system segment (eg. a Task State Segment). If set (1) it defines a code or data segment.
			 */
			enum class SegmentType : u8
			{
				SystemSegment = 0,
				CodeDataSegment = 1,
			} type : 1;

			/**
			 * E: Executable bit. If clear (0) the descriptor defines a data segment. If set (1) it defines a code segment which can be executed from.
			 */
			u8 executable : 1;

			/**
			 * DC: Direction bit/Conforming bit.
				For data selectors: Direction bit. If clear (0) the segment grows up. If set (1) the segment grows down, ie. the Offset has to be greater than the Limit.
				For code selectors: Conforming bit.
					If clear (0) code in this segment can only be executed from the ring set in DPL.
					If set (1) code in this segment can be executed from an equal or lower privilege level. For example, code in ring 3 can far-jump to conforming code in a ring 2 segment. The DPL field represent the highest privilege level that is allowed to execute the segment. For example, code in ring 0 cannot far-jump to a conforming code segment where DPL is 2, while code in ring 2 and 3 can. Note that the privilege level remains the same, ie. a far-jump from ring 3 to a segment with a DPL of 2 remains in ring 3 after the jump.
			 */
			union
			{
				enum class Direction : u8
				{
					GrowsUp = 0,
					GrowsDown = 1,
				} dataSegment __attribute__((packed)) : 1;
				enum class Conforming : u8
				{
					RingEqual = 0,
					RingEqualOrLower = 1,
				} codeSegment __attribute__((packed)) : 1;
			} access __attribute__((packed));

			/**
			 * RW: Readable bit/Writable bit.
				For code segments: Readable bit. If clear (0), read access for this segment is not allowed. If set (1) read access is allowed. Write access is never allowed for code segments.
				For data segments: Writeable bit. If clear (0), write access for this segment is not allowed. If set (1) write access is allowed. Read access is always allowed for data segments.
			 */
			union
			{
				enum class Readable : u8
				{
					ExecutableOnly = 0,
					Readable = 1,
				} codeSegment __attribute__((packed)) : 1;
				enum class Writable : u8
				{
					ReadOnly = 0,
					ReadWrite = 1,
				} dataSegment __attribute__((packed)) : 1;
			} attributes __attribute__((packed));

			/**
			 * A: Accessed bit. Best left clear (0), the CPU will set it when the segment is accessed.
			 */
			u8 _accessed : 1;
		} __attribute__((packed));

		struct GDTEntry
		{
			u16 limit_0_15;
			u16 base_0_15;
			u8 base_16_23;
			u8 flags;
			u8 limit_16_19;
			u8 base_24_31;

			// .limit_0_15 resw 1,
			// .base_0_15 resw 1,
			// .base_16_23 resb 1,
			// .flags resb 1,
			// .limit_16_19_attributes resb 1,
			// .base_24_31 resb 1
		} __attribute__((packed));

		bool Init();
	}
}