#define ISR(name) __attribute__((interrupt)) void ISR_##name(void*)

namespace Interrupt
{
	typedef void(*ISR)(void*);
	typedef void(*ISR_Exception)(void*, u32);

	bool Init();

	void Register(u8 index, Interrupt::ISR isr); // TODO: returning previous handler, so chaining is possible?
	void Unregister(u8 index);
	void Unregister(Interrupt::ISR isr);

	/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;
	; IRQs list
	;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

	const u8 IRQ_TIMER			=  0;
	const u8 IRQ_KEYBOARD		=  1;
	const u8 IRQ_UNUSED			=  2;
	const u8 IRQ_COM2			=  3;
	const u8 IRQ_COM1			=  4;
	const u8 IRQ_LPT2			=  5;
	const u8 IRQ_FLOPPY			=  6;
	const u8 IRQ_LPT1			=  7;
	const u8 IRQ_RTC			=  8;
	const u8 IRQ_PERIPHERAL1	=  9;
	const u8 IRQ_PERIPHERAL2	= 10;
	const u8 IRQ_PERIPHERAL3	= 11;
	const u8 IRQ_MOUSE			= 12;
	const u8 IRQ_FPU			= 13;
	const u8 IRQ_ATA1			= 14;
	const u8 IRQ_ATA2			= 15;

	const u8 INT_PIC0_OFFSET	= 0x20;
	const u8 INT_PIC1_OFFSET	= 0x40;

	const u8 PIC0_PORT_CMD		= 0x20;
	const u8 PIC0_PORT_DATA		= 0x21;

	const u8 PIC1_PORT_CMD		= 0xA0;
	const u8 PIC1_PORT_DATA		= 0xA1;

	const u8 PIC_EOI			= 0x20;

	const u8 INT_IRQ0			= INT_PIC0_OFFSET + 0;
	const u8 INT_IRQ1			= INT_PIC0_OFFSET + 1;
	const u8 INT_IRQ2			= INT_PIC0_OFFSET + 2;
	const u8 INT_IRQ3			= INT_PIC0_OFFSET + 3;
	const u8 INT_IRQ4			= INT_PIC0_OFFSET + 4;
	const u8 INT_IRQ5			= INT_PIC0_OFFSET + 5;
	const u8 INT_IRQ6			= INT_PIC0_OFFSET + 6;
	const u8 INT_IRQ7			= INT_PIC0_OFFSET + 7;

	const u8 INT_IRQ8			= INT_PIC1_OFFSET + 0;
	const u8 INT_IRQ9			= INT_PIC1_OFFSET + 1;
	const u8 INT_IRQ10			= INT_PIC1_OFFSET + 2;
	const u8 INT_IRQ11			= INT_PIC1_OFFSET + 3;
	const u8 INT_IRQ12			= INT_PIC1_OFFSET + 4;
	const u8 INT_IRQ13			= INT_PIC1_OFFSET + 5;
	const u8 INT_IRQ14			= INT_PIC1_OFFSET + 6;
	const u8 INT_IRQ15			= INT_PIC1_OFFSET + 7;

	/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;
	; CPU exceptions list
	;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

	const u8 INT_DIVISION_BY_ZERO				= 0x00;
	const u8 INT_SINGLE_STEP					= 0x01;
	const u8 INT_NONMASKABLE					= 0x02;
	const u8 INT_BREAKPOINT						= 0x03;
	const u8 INT_OVERFLOW						= 0x04;
	const u8 INT_BOUNDS							= 0x05;
	const u8 INT_INVALID_OPCODE					= 0x06;
	const u8 INT_NO_FPU							= 0x07;
	const u8 INT_DOUBLE_FAULT					= 0x08;
	const u8 INT_FPU_SEGMENT_OVERRUN			= 0x09;
	const u8 INT_INVALID_TASK_STATE_SEGMENT		= 0x0A;
	const u8 INT_INVALID_SEGMENT				= 0x0B;
	const u8 INT_STACK_FAULT					= 0x0C;
	const u8 INT_GENERAL_PROTECTION_FAULT		= 0x0D;
	const u8 INT_PAGE_FAULT						= 0x0E;
	const u8 INT_RESERVED						= 0x0F;
	const u8 INT_MATH_FAULT						= 0x10;
	const u8 INT_ALIGNMENT_CHECK				= 0x11;
	const u8 INT_MACHINE_CHECK					= 0x12;
	const u8 INT_FPU_EXCEPTION					= 0x13;
	const u8 INT_VIRTUALIZATION_EXCEPTION		= 0x14;
	const u8 INT_CONTROL_PROTECTION_EXCEPTION	= 0x15;
	const u8 INT_SECURITY_EXCEPTION				= 0x1E;

	constexpr u8 IRQ2INT(u8 irq)
	{
		if(/*irq >= 0 &&*/ irq < 8)
			return INT_PIC0_OFFSET + irq;
		else if(irq >= 8 && irq < 16)
			return INT_PIC1_OFFSET + irq - 8;
		else
			return 0xFF;
	}

	inline void AckIRQ()
	{
		__asm(
		"mov $0x20, %%dx \r\n"
		"mov $0x20, %%al \r\n"
		"out %%al, %%dx \r\n"
		: 
		:
		: "eax", "edx");
	}

	void Enable();
	void Disable();
	bool IsEnabled();
}
