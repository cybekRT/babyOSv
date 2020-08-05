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

	Interrupt::ISR GetAddress()
	{
		return (Interrupt::ISR)((address_16_31 << 16) | address_0_15);
	}
} __attribute__((packed));

#define PIC_CMD_INIT	0x11

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

struct IDT
{
	IDT_Entry entries[256];
} __attribute__((packed));

struct IDT_Handle
{
	u16 size;
	IDT* address;
} __attribute__((packed));

u8 HAL_In(u16 port);
void HAL_Out(u16 port, u8 data);

namespace Interrupt
{
	IDT_Handle idtHandle;
	IDT* idt;
	void* idtPhysical;

	__attribute__ ((interrupt))
	void ISR_Dummy(void* ptr)
	{
		PutString("ISR: ");
		PutHex((unsigned)ptr);
		PutString("\n");
	}

	__attribute__ ((interrupt))
	void ISR_IRQ_Dummy(void* ptr)
	{
		PutString("ISR: ");
		PutHex((unsigned)ptr);
		PutString("\n");

		HAL_Out(0x20, 0x20);
	}

	__attribute__ ((interrupt))
	void ISR_GPF(void* ptr)
	{
		PutString("\n===== General Protection Fault =====\n");

		u32* p = (u32*)ptr;
		PutHex(p[0]); PutString("\n");
		PutHex(p[1]); PutString("\n");
		PutHex(p[2]); PutString("\n");
		PutHex(p[3]); PutString("\n");
		PutHex(p[4]); PutString("\n");
		PutHex(p[5]); PutString("\n");
		PutHex(p[6]); PutString("\n");
		PutHex(p[7]); PutString("\n");
		PutHex(p[8]); PutString("\n");

		PutHex((u32)ptr);

		HALT
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

		for(unsigned a = 0; a < 256; a++)
		{
			idt->entries[a].address_0_15 = 0;
			idt->entries[a].address_16_31 = 0;
			idt->entries[a].selector = 0x8;
			idt->entries[a].unused = 0;
			idt->entries[a].flags = IDT_Entry::Flag::IDT_FLAG_32BIT_INT_GATE;
		}

		idtHandle.size = sizeof(IDT) - 1;
		idtHandle.address = (IDT*)idt;

		__asm(
			"lidt (%0)"
		: : "a"(&idtHandle));

		PutString("IDT handle: ");
		PutHex((unsigned)(&idtHandle));
		PutString("\n");

		HAL_Out(PIC0_PORT_CMD, PIC_CMD_INIT);
		HAL_Out(PIC0_PORT_DATA, INT_PIC0_OFFSET);
		HAL_Out(PIC0_PORT_DATA, ICW1_INTERVAL4);
		HAL_Out(PIC0_PORT_DATA, ICW4_8086);
		HAL_Out(PIC1_PORT_CMD, PIC_CMD_INIT);
		HAL_Out(PIC1_PORT_DATA, INT_PIC1_OFFSET);
		HAL_Out(PIC1_PORT_DATA, ICW1_SINGLE);
		HAL_Out(PIC1_PORT_DATA, ICW4_8086);

		/*idt->entries[0].SetAddress(Dummy);
		int v = int(IDT_Entry::Flag::IDT_FLAG_32BIT_INT_GATE) | int(IDT_Entry::Flag::IDT_FLAG_ENTRY_PRESENT);
		idt->entries[0].flags = (IDT_Entry::Flag)v;
		idt->entries[0].selector = 0x8;*/

		for(unsigned a = 0; a < 256; a++)
		{
			//isr->entries[a].SetAddress();
			//Register(a, (ISR)a);
		}

		//Register(0, Dummy);
		//Register(INT_GENERAL_PROTECTION_FAULT, ISR_GPF);
		Register(INT_DOUBLE_FAULT, ISR_GPF);
		Register(INT_PAGE_FAULT, ISR_GPF);
		//Register(INT_DIVISION_BY_ZERO, ISR_GPF);

		Register(IRQ2INT(IRQ_TIMER), ISR_IRQ_Dummy);
		Register(IRQ2INT(IRQ_LPT1), ISR_IRQ_Dummy);
		__asm("sti");
		//for(;;);

		PutString("Flags: ");
		PutHex((unsigned)idt->entries[0].flags);
		PutString("\n");
		//HALT

		//__asm("sti");
		//__asm("int $0");

		return true;
	}

	void Register(u8 index, Interrupt::ISR isr)
	{
		PutString("Registering "); PutHex((u32)index); PutString(" with handler: "); PutHex((u32)isr); PutString("\n");

		idt->entries[index].SetAddress(isr);
		idt->entries[index].flags |= IDT_Entry::Flag::IDT_FLAG_ENTRY_PRESENT;
	}

	void Unregister(u8 index)
	{
		idt->entries[index].SetAddress(nullptr);
		idt->entries[index].flags &= ~IDT_Entry::Flag::IDT_FLAG_ENTRY_PRESENT;
	}

	void Unregister(Interrupt::ISR isr)
	{
		for(unsigned a = 0; a < 256; a++)
		{
			if(idt->entries[a].GetAddress() == isr)
			{
				idt->entries[a].SetAddress(nullptr);
				idt->entries[a].flags &= ~IDT_Entry::Flag::IDT_FLAG_ENTRY_PRESENT;
				return;
			}
		}
	}
}
