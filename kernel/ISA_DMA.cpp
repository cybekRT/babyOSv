#include"ISA_DMA.hpp"
#include"HAL.h"

namespace ISA_DMA
{
	u8 PortIn(IOPort port)
	{
		return HAL::In((u16)port);
	}

	void PortOut(IOPort port, u8 data)
	{
		HAL::Out((u16)port, data);
	}

	void ResetFlipFlop()
	{
		PortOut(IOPort::FlipFlopResetRegister, 0xff);
	}

	bool Init()
	{
		ASSERT(sizeof(DMAModeRegister) == 1, "Invalid DMAModeRegister size");

		DMAModeRegister reg;
		u8* regPtr = (u8*)&reg;
		*regPtr = PortIn(IOPort::ModeRegister);

		Print("Mode reg: %x %x %x %x %x\n", reg.channel, reg.mode, reg.type, reg.reverse, reg.autoInit);
		Print("Mode reg: %x\n", *regPtr);

		reg.channel = 3;
		reg.mode = TransferMode::Block;
		reg.reverse = 1;
		reg.autoInit = 1;
		reg.type = TransferType::MemoryToPeripheral;
		Print("Mode reg2: %x\n", *regPtr);

		return true;
	}

	void Mask(u8 channel)
	{
		ASSERT(channel < 4, "Invalid channel");

		u8 data = (1 << 2) | channel;
		PortOut(IOPort::SingleChannelMaskRegister, data);
	}

	void Unmask(u8 channel)
	{
		ASSERT(channel < 4, "Invalid channel");

		u8 data = (0 << 2) | channel;
		PortOut(IOPort::SingleChannelMaskRegister, data);
	}

	void Start(u8 channel, TransferDir dir, void* physAddress, u16 count)
	{
		ASSERT((u32)physAddress <= 0xffff, "Invalid DMA address");
		u16 addr = (u16)(u32)physAddress;

		Mask(channel);

		ResetFlipFlop();

		PortOut(IOPort::StartAddressRegisterChannel2, addr & 0xff);
		PortOut(IOPort::StartAddressRegisterChannel2, addr >> 8);

		ResetFlipFlop();

		PortOut(IOPort::CountRegisterChannel2, count & 0xff);
		PortOut(IOPort::CountRegisterChannel2, count >> 8);

		PortOut(IOPort::PageAddressRegisterChannel2, 0x00);

		Unmask(channel);
	}
}