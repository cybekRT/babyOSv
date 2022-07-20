#include"Memory_GDT.hpp"

namespace Memory::GDT
{
	GDTEntry entries[3] = {
		[0] = {
			.value = 0
		},
		[1] = { .fields = {
			.limit_0_15 = 0xffff,
			.base_0_15 = 0,
			.base_16_23 = 0,
			.flags = {
				.present = 1,
				.ring = GDTEntry_Access::Ring::Ring0,
				.type = GDTEntry_Access::SegmentType::CodeDataSegment,
				.executable = true,
				.directionConforming = GDTEntry_Access::DirectionConforming::Code_RingEqual,
				.attributes = GDTEntry_Access::Readable::Code_ExecutableOnly,
				._accessed = false
			},
			.limit_16_19 = 0xf,
			.base_24_31 = 0
		}},
		[2] = { .fields ={
			.limit_0_15 = 0xffff,
			.base_0_15 = 0,
			.base_16_23 = 0,
			.flags = {
				.present = 1,
				.ring = GDTEntry_Access::Ring::Ring0,
				.type = GDTEntry_Access::SegmentType::CodeDataSegment,
				.executable = true,
				.directionConforming = GDTEntry_Access::DirectionConforming::Code_RingEqual,
				.attributes = GDTEntry_Access::Readable::Code_ExecutableOnly,
				._accessed = false
			},
			.limit_16_19 = 0xf,
			.base_24_31 = 0
		}}
	};

	GDTArray gdtInfo = {
		.length = sizeof(entries),
		.ptr = entries
	};

	bool Init()
	{
		Print("GDT~! %d\n", sizeof(GDTEntry));
		Print("      %d\n", sizeof(GDTEntry_Access));

		__asm("lgdt (%0)" : : "r"(&gdtInfo));

		return true;
	}
}