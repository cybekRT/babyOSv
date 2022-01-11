#include"Interrupt.hpp"
#include"Memory.hpp"
#include"HAL.hpp"
#include"Thread.hpp"
#include"preinit.hpp"

using Thread::currentThread;

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

	void SetAddress(Interrupt::ISR isrAddr)
	{
		u32 a = (u32)isrAddr;
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

namespace Interrupt
{
	IDT_Handle idtHandle;
	IDT* idt;
	void* idtPhysical;

	ISR(Dummy)
	{
		/*PutString("ISR: ");
		PutHex((unsigned)ptr);
		PutString("\n");*/
	}

	ISR(IRQ_Dummy)
	{
		/*PutString("ISR: ");
		PutHex((unsigned)ptr);
		PutString("\n");*/

		//HAL::Out8(0x20, 0x20);
		Interrupt::AckIRQ();
	}

	struct ISR_Registers
	{
		u32 eip;
		u32 cs;
		u32 eflags;

		u32 eax;
		u32 ebx;
		u32 ecx;
		u32 edx;
		u32 esi;
		u32 edi;
		u32 esp;
		u32 ebp;

		u16 ds;
		u16 es;
		u16 fs;
		u16 gs;
		u16 ss;
	};

	void PrintCallstack()
	{
		u32* ebp;
		__asm("mov %%ebp, %0" : "=m"(ebp));
		Print("Callstack:\n");
		while(ebp)
		{
			u32 addr = 0;
			for(u32 a = 0; a < 8; a++)
			{
				if(ebp[a] >= _kernel_code_beg && ebp[a] <= _kernel_code_end)
				{
					addr = ebp[a];
					break;
				}
			}

			Print(" - %p\n", addr);

			ebp = (u32*)(*ebp);
		}
	}

	__attribute__((naked, noreturn))
	void ISR_DoubleFault(void* ptr)
	{
		__asm("cli");
		__asm("xchg %bx, %bx");
		for(;;)
		{
			__asm("hlt");
		}
	}

	static bool insidePageFault = false;
	__attribute__((naked))
	void ISR_PageFault(void* ptr)
	{
		__asm("cli");
		__asm("xchg %bx, %bx");

		if(insidePageFault)
		{
			Print("WTF~!\n");
			for(;;)
				HALT;
		}

		insidePageFault = true;

		void* faultAddr;
		__asm(
			"mov %%cr2, %%eax \n"
			"mov %%eax, %0"
			: "=r"(faultAddr));
		Terminal::Print("Page fault: %p\n", faultAddr);

		PrintCallstack();

		u32 ebp, esp;
		__asm("mov %%esp, %0" : "=r"(esp));
		__asm("mov %%ebp, %0" : "=r"(ebp));

		Print("EBP: %x, ESP: %x\n", ebp, esp);

		u32* x = ((u32*)&ptr)-6;
		Print("(%p)\n", x);
		for(int a = 0; a < 8; a++)
		{
			// if(*x != 0)
				Print("_ [%p] = %p\n", x, *x);
			x++;
		}

		for(;;)
			HALT;
	}

	__attribute__ ((interrupt, noreturn))
	void ISR_GPF(ISR_Registers* _registers, u32 errorCode)
	{
		static ISR_Registers regs;
		__asm("cli");

		__asm("mov %%eax, %0" : "=m"(regs.eax));
		__asm("mov %%ebx, %0" : "=m"(regs.ebx));
		__asm("mov %%ecx, %0" : "=m"(regs.ecx));
		__asm("mov %%edx, %0" : "=m"(regs.edx));
		__asm("mov %%esi, %0" : "=m"(regs.esi));
		__asm("mov %%edi, %0" : "=m"(regs.edi));
		__asm("mov %%esp, %0" : "=m"(regs.esp));
		__asm("mov %%ebp, %0" : "=m"(regs.ebp));

		__asm("mov %%ds, %%bx \n mov %%bx, %0" : "=m"(regs.ds) : : "bx");
		__asm("mov %%es, %%bx \n mov %%bx, %0" : "=m"(regs.es) : : "bx");
		__asm("mov %%fs, %%bx \n mov %%bx, %0" : "=m"(regs.fs) : : "bx");
		__asm("mov %%gs, %%bx \n mov %%bx, %0" : "=m"(regs.gs) : : "bx");
		__asm("mov %%ss, %%bx \n mov %%bx, %0" : "=m"(regs.ss) : : "bx");

		regs.cs = _registers->cs;
		regs.eip = _registers->eip;
		regs.eflags = _registers->eflags;

		PutString("\n");
		Print("===== %s =====\n", Thread::currentThread->name);
		PutString("===== General Protection Fault =====");
		PutString("\n");

		Print("Address: %x:%x\n", regs.cs, regs.eip);
		Print("Flags:   %x\n", regs.eflags);
		Print("Error code: %s%s - %x\n",
			(errorCode & 1 ? "(EXT) " : ""),
			((errorCode >> 1) & 0b11) == 0 ? "GDT" : ((errorCode >> 1) & 0b11) == 0b10 ? "LDT" : "IDT",
			((errorCode >> 3) & 0b1111111111111) );
		Print("Stack:   %x:%x, %x\n", regs.ss, regs.esp, regs.ebp);

		Print("EAX: %x, EBX: %x, ECX: %x, EDX: %x\n", regs.eax, regs.ebx, regs.ecx, regs.edx);
		Print("ESI: %x, EDI: %x\n", regs.esi, regs.edi);
		Print(" DS: %x,  ES: %x,  FS: %x,  GS: %x\n", regs.ds, regs.es, regs.fs, regs.gs);

		PrintCallstack();

		__asm("cli");
		for(;;)
		{
			HALT;
		}
	}

	bool Init()
	{
		ASSERT(sizeof(IDT_Entry) == 8, "idt_entry size");
		ASSERT(sizeof(IDT_Handle) == 6, "idt_handle size");

		PutString("Initializing IDT...\n");

		idtPhysical = Memory::Physical::Alloc(sizeof(IDT));
		idt = (IDT*)Memory::Logical::Map(idtPhysical, nullptr, sizeof(IDT));

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

		HAL::Out8(PIC0_PORT_CMD, PIC_CMD_INIT);
		HAL::Out8(PIC0_PORT_DATA, INT_PIC0_OFFSET);
		HAL::Out8(PIC0_PORT_DATA, ICW1_INTERVAL4);
		HAL::Out8(PIC0_PORT_DATA, ICW4_8086);
		HAL::Out8(PIC1_PORT_CMD, PIC_CMD_INIT);
		HAL::Out8(PIC1_PORT_DATA, INT_PIC1_OFFSET);
		HAL::Out8(PIC1_PORT_DATA, ICW1_SINGLE);
		HAL::Out8(PIC1_PORT_DATA, ICW4_8086);

		Register(INT_GENERAL_PROTECTION_FAULT, (ISR)ISR_GPF);
		Register(INT_DOUBLE_FAULT, (ISR)ISR_DoubleFault);
		Register(INT_PAGE_FAULT, (ISR)ISR_PageFault);
		Register(INT_INVALID_SEGMENT, (ISR)ISR_GPF);
		//Register(INT_DIVISION_BY_ZERO, ISR_GPF);

		/*for(unsigned a = 0; a < 16; a++)
		{
			Register(IRQ2INT(a), ISR_IRQ_Dummy);
		}*/

		//Register(IRQ2INT(IRQ_TIMER), ISR_IRQ_Dummy);
		Register(IRQ2INT(IRQ_LPT1), ISR_IRQ_Dummy);
		//__asm("sti");

		return true;
	}

	void Register(u8 index, Interrupt::ISR isr)
	{
		Print("Registering %x  with handler: %p... ", (u32)isr);

		idt->entries[index].SetAddress(isr);
		idt->entries[index].flags |= IDT_Entry::Flag::IDT_FLAG_ENTRY_PRESENT;

		Print("Ok~!\n");
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

	// u32 disableCount = 1;

	void Enable()
	{
		if(!currentThread)
		{
			// __asm("sti");
			return;
		}

		ASSERT(currentThread->interruptDisabled > 0, "Interrupts disableCount == 0");

		__asm("pushf\ncli");
		currentThread->interruptDisabled--;
		if(currentThread->interruptDisabled == 0)
		{
			__asm("popf");
			__asm("sti");
		}
		else
			__asm("popf");
	}

	void Disable()
	{
		if(!currentThread)
		{
			// __asm("cli");
			return;
		}

		// if(currentThread->interruptDisabled == 0)
		// {
			__asm("cli");
		// }

		currentThread->interruptDisabled++;
	}

	bool IsEnabled()
	{
		u32 v;
		__asm("pushf \r\n pop %0" : "=r"(v) : : "memory");

		return !!(v & 0x200);
	}
}
