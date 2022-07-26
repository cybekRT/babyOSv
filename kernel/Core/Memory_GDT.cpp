#include"Memory_GDT.hpp"

namespace Memory::GDT
{
	__attribute__((aligned(8)))
	static GDTEntry entries[3] = {
		[0] = {
			.value = 0
		},
		[1] = { .fields = {
			.limit_0_15 = 0xffff,
			.base_0_15 = 0,
			.base_16_23 = 0,
			.flags = {
				._accessed = false,
				.attributes = GDTEntry_Access::Readable::Code_Readable,
				.directionConforming = GDTEntry_Access::DirectionConforming::Code_RingEqualOrLower,
				.executable = true,
				.type = GDTEntry_Access::SegmentType::CodeDataSegment,
				.ring = GDTEntry_Access::Ring::Ring0,
				.present = 1,
			},
			._reserved = 0,
			.longMode = false,
			.sizeFlag = SizeFlag::Protected32b,
			.limitType = LimitType::Block4k,
			.limit_16_19 = 0xf,
			.base_24_31 = 0
		}},
		[2] = { .fields ={
			.limit_0_15 = 0xffff,
			.base_0_15 = 0,
			.base_16_23 = 0,
			.flags = {
				._accessed = false,
				.attributes = GDTEntry_Access::Readable::Data_ReadWrite,
				.directionConforming = GDTEntry_Access::DirectionConforming::Data_GrowsUp,
				.executable = false,
				.type = GDTEntry_Access::SegmentType::CodeDataSegment,
				.ring = GDTEntry_Access::Ring::Ring0,
				.present = 1,
			},
			._reserved = 0,
			.longMode = false,
			.sizeFlag = SizeFlag::Protected32b,
			.limitType = LimitType::Block4k,
			.limit_16_19 = 0xf,
			.base_24_31 = 0
		}}
	};

	__attribute__((aligned(8)))
	static GDTArray gdtInfo = {
		.length = sizeof(entries),
		.ptr = entries
	};

	bool Init()
	{
		// FIXME: if uncommented, then everything work :/
		// Print("GDT~! %d\n", sizeof(GDTEntry));
		// Print("      %d\n", sizeof(GDTEntry_Access));
		// Print("      %d\n", sizeof(gdtInfo));
		// Print("Addr: %p\n", entries);
		// Print("Addr: %p\n", &gdtInfo);
		// Print("GDT: %x%x, %x%x, %x%x\n", entries[0].value, entries[1].value, entries[2].value);

		__asm("xchg %bx, %bx");
		__asm("lgdt (%0)" : : "r"(&gdtInfo));
		__asm("pushw $0x8");
		__asm("push $label\r\n"
		"retf\r\n"
		"label:");

		return true;
	}
}