#include"Interrupt.h"
#include"Memory.h"

struct IDT_Entry
{
	enum Flag : u8
	{
		IDT_FLAG_32BIT_TASK_GATE = (0x5 << 0),
		IDT_FLAG_16BIT_INT_GATE = (0x6 << 0),
		IDT_FLAG_16BIT_TRAP_GATE = (0x7 << 0),
		IDT_FLAG_32BIT_INT_GATE = (0xE << 0),
		IDT_FLAG_32BIT_TRAP_GATE = (0xF << 0),

		IDT_FLAG_STORAGE_SEGMENT = (0 << 4),
		IDT_FLAG_RING_0 = (0b00 << 5),
		IDT_FLAG_RING_1 = (0b01 << 5),
		IDT_FLAG_RING_2 = (0b10 << 5),
		IDT_FLAG_RING_3 = (0b11 << 5),
		IDT_FLAG_ENTRY_PRESENT = (1 << 7)
	};

	u16 address_0_15;
	u16 selector;
	u8 unused;
	u8 flags;
	u16 address_16_31;

	void SetAddress(void (*addr)(void*))
	{
		u32 a = (u32)addr;
		address_0_15 = a & 0xFFFF;
		address_16_31 = a >> 16;
	}
} __attribute__((packed));

struct IDT
{
	//void (*handlers[256])();
	IDT_Entry entries[256];
} __attribute__((packed));

struct IDT_Handle
{
	u16 size;
	IDT* address;
} __attribute__((packed));

namespace Interrupt
{
	IDT_Handle idtHandle;
	IDT* idt;
	void* idtPhysical;

	__attribute__ ((interrupt))
	void Dummy(void* ptr)
	{
		PutString("ISR: ");
		PutHex((unsigned)ptr);
		PutString("\n");
	}

	bool Init()
	{
		ASSERT(sizeof(IDT_Entry) == 8, "idt_entry size");
		ASSERT(sizeof(IDT_Handle) == 6, "idt_handle size");

		PutString("Initializing IDT...\n");

		idtPhysical = Memory::AllocPhys(sizeof(IDT));
		idt = (IDT*)Memory::Map(idtPhysical, nullptr, sizeof(IDT));

		if(!idtPhysical || !idt)
			return false;

		PutHex((unsigned)idt);

		for(unsigned a = 0; a < sizeof(*idt); a++)
		{
			u8* p = (u8*)idt;
			p[a] = 0;
		}

		idtHandle.size = sizeof(IDT) - 1;
		idtHandle.address = (IDT*)idt;

		__asm(
			"lidt (%0)"
		: : "a"(&idtHandle));

		PutString("IDT handle: ");
		PutHex((unsigned)(&idtHandle));
		PutString("\n");

		idt->entries[0].SetAddress(Dummy);
		int v = int(IDT_Entry::Flag::IDT_FLAG_32BIT_INT_GATE) | int(IDT_Entry::Flag::IDT_FLAG_ENTRY_PRESENT);
		idt->entries[0].flags = (IDT_Entry::Flag)v;
		idt->entries[0].selector = 0x8;

		PutString("Flags: ");
		PutHex((unsigned)idt->entries[0].flags);
		PutString("\n");
		//HALT

		//__asm("sti");
BREAK
		__asm("int $0");

		return true;
	}

	void Register(u8 index, void(*handler)())
	{

	}

	void Unregister(u8 index)
	{

	}
}
