#include"Memory_GDT.hpp"
#include"Memory.hpp"

namespace Memory::GDT
{
	__attribute__((aligned(8)))
	GDTEntry entries[] =
	{
		[0] =
		{
			.value = 0
		},
		[1] =
		{
			.fields =
			{
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
			}
		},
		[2] =
		{
			.fields =
			{
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
			}
		},
		[3] =
		{
			.fields =
			{
				.limit_0_15 = 0xffff,
				.base_0_15 = 0,
				.base_16_23 = 0,
				.flags = {
					._accessed = false,
					.attributes = GDTEntry_Access::Readable::Code_ExecutableOnly,
					.directionConforming = GDTEntry_Access::DirectionConforming::Code_RingEqual,
					.executable = true,
					.type = GDTEntry_Access::SegmentType::CodeDataSegment,
					.ring = GDTEntry_Access::Ring::Ring3,
					.present = 1,
				},
				._reserved = 0,
				.longMode = false,
				.sizeFlag = SizeFlag::Protected32b,
				.limitType = LimitType::Block4k,
				.limit_16_19 = 0xf,
				.base_24_31 = 0
			}
		},
		[4] =
		{
			.fields =
			{
				.limit_0_15 = 0xffff,
				.base_0_15 = 0,
				.base_16_23 = 0,
				.flags = {
					._accessed = false,
					.attributes = GDTEntry_Access::Readable::Code_ExecutableOnly,
					.directionConforming = GDTEntry_Access::DirectionConforming::Code_RingEqual,
					.executable = true,
					.type = GDTEntry_Access::SegmentType::CodeDataSegment,
					.ring = GDTEntry_Access::Ring::Ring3,
					.present = 1,
				},
				._reserved = 0,
				.longMode = false,
				.sizeFlag = SizeFlag::Protected32b,
				.limitType = LimitType::Block4k,
				.limit_16_19 = 0xf,
				.base_24_31 = 0
			}
		},
		// TSS
		[5] =
		{
			.fields =
			{
				.limit_0_15 = 0,
				.base_0_15 = 0,
				.base_16_23 = 0,
				.flags = {
					._accessed = false,
					.attributes = GDTEntry_Access::Readable::Code_ExecutableOnly,
					.directionConforming = GDTEntry_Access::DirectionConforming::Data_GrowsUp,
					.executable = true,
					.type = GDTEntry_Access::SegmentType::SystemSegment,
					.ring = GDTEntry_Access::Ring::Ring0,
					.present = 1,
				},
				._reserved = 0,
				.longMode = false,
				.sizeFlag = SizeFlag::Protected32b,
				.limitType = LimitType::Byte,
				.limit_16_19 = 0,
				.base_24_31 = 0
			}
		}
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

		// for(unsigned a = 0; a < 0xffffff; a++)
		// 	__asm("nop");

		Reload();

		return true;
	}

	void Reload()
	{
		__asm("lgdt (%0)" : : "r"(&gdtInfo));
		__asm("push $0x8");
		__asm("push $label\r\n"
		"retf\r\n"
		"label:");
	}

	tss_entry_struct globalTSS;

	bool InitTSS()
	{
		memset(&globalTSS, 0, sizeof(globalTSS));

		u8* tssStack = (u8*)Memory::Alloc(8192);
		if(!tssStack)
			return false;

		globalTSS.ss0 = 0x10;
		globalTSS.esp0 = (u32)(tssStack + 8192);
		globalTSS.iomap_base = (u32)sizeof(globalTSS);

		u32 globalTSSV = (u32)(&globalTSS);
		entries[5].fields.limit_0_15 = sizeof(globalTSS) & 0xffff;
		entries[5].fields.limit_16_19 = (sizeof(globalTSS) >> 16) & 0xff;
		entries[5].fields.base_0_15 = (globalTSSV & 0xffff);
		entries[5].fields.base_16_23 = ((globalTSSV >> 16) & 0xff);
		entries[5].fields.base_24_31 = ((globalTSSV >> 24) & 0xff);

		Reload();
		__asm("xchg %bx, %bx");

		__asm(
			"mov $(5*8 | 0), %%ax \r\n"
			"ltr %%ax \r\n"
			: : : "ax"
		);

		return true;
	}
}