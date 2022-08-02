#pragma once

namespace Memory
{
	struct tss_entry_struct {
		u32 prev_tss; // The previous TSS - with hardware task switching these form a kind of backward linked list.
		u32 esp0;     // The stack pointer to load when changing to kernel mode.
		u32 ss0;      // The stack segment to load when changing to kernel mode.
		// Everything below here is unused.
		u32 esp1; // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
		u32 ss1;
		u32 esp2;
		u32 ss2;
		u32 cr3;
		u32 eip;
		u32 eflags;
		u32 eax;
		u32 ecx;
		u32 edx;
		u32 ebx;
		u32 esp;
		u32 ebp;
		u32 esi;
		u32 edi;
		u32 es;
		u32 cs;
		u32 ss;
		u32 ds;
		u32 fs;
		u32 gs;
		u32 ldt;
		u16 trap;
		u16 iomap_base;
	} __attribute__((packed));

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
			 * A: Accessed bit. Best left clear (0), the CPU will set it when the segment is accessed.
			 */
			u8 _accessed : 1;

			/**
			 * RW: Readable bit/Writable bit.
				For code segments: Readable bit. If clear (0), read access for this segment is not allowed. If set (1) read access is allowed. Write access is never allowed for code segments.
				For data segments: Writeable bit. If clear (0), write access for this segment is not allowed. If set (1) write access is allowed. Read access is always allowed for data segments.
			 */
			enum class Readable : u8
			{
				// Code segment
				Code_ExecutableOnly = 0,
				Code_Readable = 1,
				// Data segment
				Data_ReadOnly = 0,
				Data_ReadWrite = 1,
			} attributes : 1;

			/**
			 * DC: Direction bit/Conforming bit.
				For data selectors: Direction bit. If clear (0) the segment grows up. If set (1) the segment grows down, ie. the Offset has to be greater than the Limit.
				For code selectors: Conforming bit.
					If clear (0) code in this segment can only be executed from the ring set in DPL.
					If set (1) code in this segment can be executed from an equal or lower privilege level. For example, code in ring 3 can far-jump to conforming code in a ring 2 segment. The DPL field represent the highest privilege level that is allowed to execute the segment. For example, code in ring 0 cannot far-jump to a conforming code segment where DPL is 2, while code in ring 2 and 3 can. Note that the privilege level remains the same, ie. a far-jump from ring 3 to a segment with a DPL of 2 remains in ring 3 after the jump.
			 */
			enum class DirectionConforming : u8
			{
				// Data segment
				Data_GrowsUp = 0,
				Data_GrowsDown = 1,
				// Code segment
				Code_RingEqual = 0,
				Code_RingEqualOrLower = 1,
			} directionConforming : 1;

			/**
			 * E: Executable bit. If clear (0) the descriptor defines a data segment. If set (1) it defines a code segment which can be executed from.
			 */
			bool executable : 1;

			/**
			 * S: Descriptor type bit. If clear (0) the descriptor defines a system segment (eg. a Task State Segment). If set (1) it defines a code or data segment.
			 */
			enum class SegmentType : u8
			{
				SystemSegment = 0,
				CodeDataSegment = 1,
			} type : 1;

			/**
			 * DPL: Descriptor privilege level field. Contains the CPU Privilege level of the segment. 0 = highest privilege (kernel), 3 = lowest privilege (user applications).
			 */
			enum class Ring : u8
			{
				Ring0 = 0,
				Ring1 = 1,
				Ring2 = 2,
				Ring3 = 3,
			} ring : 2;

			/**
			 * P: Present bit. Allows an entry to refer to a valid segment. Must be set (1) for any valid segment.
			 */
			u8 present : 1;
		} __attribute__((packed));

		enum class SizeFlag : u8
		{
			Protected16b,
			Protected32b
		};
		enum class LimitType : u8
		{
			Byte,
			Block4k,
		};

		union GDTEntry
		{
			struct
			{
				u16 limit_0_15;
				u16 base_0_15;
				u8 base_16_23;
				GDTEntry_Access flags;
				u8 limit_16_19 : 4;
				u8 tssAvailable : 1;
				u8 longMode : 1;
				SizeFlag sizeFlag : 1;
				LimitType limitType : 1;
				u8 base_24_31;
			} fields __attribute__((packed));
			u64 value;
		} __attribute__((packed));

		struct GDTArray
		{
			u16 length;
			GDTEntry* ptr;
		} __attribute__((packed));

		bool Init();
		void Reload();
		bool InitTSS();
	}
}