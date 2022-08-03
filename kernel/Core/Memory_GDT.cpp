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
				.limit_16_19 = 0xf,
				.tssAvailable = 0,
				.longMode = false,
				.sizeFlag = SizeFlag::Protected32b,
				.limitType = LimitType::Block4k,
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
				.limit_16_19 = 0xf,
				.tssAvailable = 0,
				.longMode = false,
				.sizeFlag = SizeFlag::Protected32b,
				.limitType = LimitType::Block4k,
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
				.limit_16_19 = 0xf,
				.tssAvailable = 0,
				.longMode = false,
				.sizeFlag = SizeFlag::Protected32b,
				.limitType = LimitType::Block4k,
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
					._accessed = true,
					.attributes = GDTEntry_Access::Readable::Data_ReadWrite,
					.directionConforming = GDTEntry_Access::DirectionConforming::Data_GrowsUp,
					.executable = false,
					.type = GDTEntry_Access::SegmentType::CodeDataSegment,
					.ring = GDTEntry_Access::Ring::Ring3,
					.present = 1,
				},
				.limit_16_19 = 0xf,
				.tssAvailable = 0,
				.longMode = false,
				.sizeFlag = SizeFlag::Protected32b,
				.limitType = LimitType::Block4k,
				.base_24_31 = 0
			}
		},
		// TSS - normal
		[5] =
		{
			.fields =
			{
				.limit_0_15 = 0,
				.base_0_15 = 0,
				.base_16_23 = 0,
				.flags = {
					._accessed = true,
					.attributes = GDTEntry_Access::Readable::Code_ExecutableOnly,
					.directionConforming = GDTEntry_Access::DirectionConforming::Data_GrowsUp,
					.executable = true,
					.type = GDTEntry_Access::SegmentType::SystemSegment,
					.ring = GDTEntry_Access::Ring::Ring0,
					.present = 1,
				},
				.limit_16_19 = 0,
				.tssAvailable = true,
				.longMode = false,
				.sizeFlag = SizeFlag::Protected32b,
				.limitType = LimitType::Byte,
				.base_24_31 = 0
			}
		},
		// TSS - interrupt
		[6] =
		{
			.fields =
			{
				.limit_0_15 = 0,
				.base_0_15 = 0,
				.base_16_23 = 0,
				.flags = {
					._accessed = true,
					.attributes = GDTEntry_Access::Readable::Code_ExecutableOnly,
					.directionConforming = GDTEntry_Access::DirectionConforming::Data_GrowsUp,
					.executable = true,
					.type = GDTEntry_Access::SegmentType::SystemSegment,
					.ring = GDTEntry_Access::Ring::Ring0,
					.present = 1,
				},
				.limit_16_19 = 0,
				.tssAvailable = true,
				.longMode = false,
				.sizeFlag = SizeFlag::Protected32b,
				.limitType = LimitType::Byte,
				.base_24_31 = 0
			}
		},
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

	__attribute__((aligned(4)))
	tss_entry_struct globalTSS;
	__attribute__((aligned(4)))
	tss_entry_struct globalTSSInt;

	struct gdt_entry_bits
	{
		u32 limit_low              : 16;
		u32 base_low               : 24;
		u32 accessed               :  1;
		u32 read_write             :  1; // readable for code, writable for data
		u32 conforming_expand_down :  1; // conforming for code, expand down for data
		u32 code                   :  1; // 1 for code, 0 for data
		u32 code_data_segment      :  1; // should be 1 for everything but TSS and LDT
		u32 DPL                    :  2; // privilege level
		u32 present                :  1;
		u32 limit_high             :  4;
		u32 available              :  1; // only used in software; has no effect on hardware
		u32 long_mode              :  1;
		u32 big                    :  1; // 32-bit opcodes for code, uint32_t stack for data
		u32 gran                   :  1; // 1 to use 4k page addressing, 0 for byte addressing
		u32 base_high              :  8;
	} __attribute__((packed));

	void YoLo()
	{
		u32 addr;

		__asm("mov %%cr2, %0" : "=r"(addr));
		Print("Page fault: %x", addr);

		for(;;);
	}

	bool InitTSS()
	{
		// TSS 0x28
		memset(&globalTSS, 0, sizeof(globalTSS));

		u8* tssStack = (u8*)Memory::Alloc(8192);
		if(!tssStack)
			return false;

		globalTSS.ss0 = 0x10;
		globalTSS.esp0 = (u32)(tssStack + 8192);
		globalTSS.iomap_base = (u32)sizeof(globalTSS);
		globalTSS.cs = 0x08;
		globalTSS.eip = (u32)YoLo;
		u32 globalTSSV = (u32)(&globalTSS);
		entries[5].fields.limit_0_15 = sizeof(globalTSS) & 0xffff;
		entries[5].fields.limit_16_19 = (sizeof(globalTSS) >> 16) & 0xff;
		entries[5].fields.base_0_15 = (globalTSSV & 0xffff);
		entries[5].fields.base_16_23 = ((globalTSSV >> 16) & 0xff);
		entries[5].fields.base_24_31 = ((globalTSSV >> 24) & 0xff);

		// TSS 0x30 - interrupt
		memset(&globalTSS, 0, sizeof(globalTSS));

		tssStack = (u8*)Memory::Alloc(2048);
		if(!tssStack)
			return false;

		globalTSSInt.ss = 0x10;
		globalTSSInt.esp = (u32)(tssStack + 8192);
		globalTSSInt.iomap_base = (u32)sizeof(globalTSSInt);
		__asm("mov %%cr3, %0" : "=r"(globalTSSInt.cr3));
		globalTSSInt.cs = 0x08;
		globalTSSInt.eip = (u32)YoLo;
		globalTSSV = (u32)(&globalTSSInt);
		entries[6].fields.limit_0_15 = sizeof(globalTSSInt) & 0xffff;
		entries[6].fields.limit_16_19 = (sizeof(globalTSSInt) >> 16) & 0xff;
		entries[6].fields.base_0_15 = (globalTSSV & 0xffff);
		entries[6].fields.base_16_23 = ((globalTSSV >> 16) & 0xff);
		entries[6].fields.base_24_31 = ((globalTSSV >> 24) & 0xff);


		// Reload GDT and load TSS
		Reload();
		Print("TSS value: %x %x\n", entries[5].value);

		// __asm("xchg %bx, %bx");

		__asm(
			"mov $(5*8 | 0), %%ax \r\n"
			"ltr %%ax \r\n"
			: : : "ax"
		);

		return true;
	}
}