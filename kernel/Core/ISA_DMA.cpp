#include"ISA_DMA.hpp"
#include"HAL.hpp"

namespace ISA_DMA
{
	u8 PortIn(IOPort port)
	{
		return HAL::In8((u16)port);
	}

	void PortOut(IOPort port, u8 data)
	{
		HAL::Out8((u16)port, data);
	}

	void ResetFlipFlop()
	{
		PortOut(IOPort::FlipFlopResetRegister, 0xff);
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

	bool Init()
	{
		ASSERT(sizeof(DMAModeRegister) == 1, "Invalid DMAModeRegister size");

		return true;
	}

	void Start(u8 channel, TransferDir dir, void* physAddress, u16 count)
	{
		// TODO: "write" direction
		ASSERT(channel == 2, "Only channel 2 is supported~!");
		u32 addr = (u32)physAddress;

		ASSERT((addr & 0xffff) + count <= 0xffff, "Invalid DMA address");

		Mask(channel);

		ResetFlipFlop();

		PortOut(IOPort::StartAddressRegisterChannel2, addr & 0xff);
		PortOut(IOPort::StartAddressRegisterChannel2, addr >> 8);
		PortOut(IOPort::PageAddressRegisterChannel2, addr >> 16);

		ResetFlipFlop();

		PortOut(IOPort::CountRegisterChannel2, count & 0xff);
		PortOut(IOPort::CountRegisterChannel2, count >> 8);		

		// Unmask(channel);
		// Mask(channel);

		if(dir == TransferDir::READ)
		{
			DMAModeRegister reg;
			u8* regPtr = (u8*)&reg;
			reg.channel = 2;
			reg.mode = TransferMode::Single;
			reg.reverse = false;
			reg.autoInit = false;
			reg.type = TransferType::PeripheralToMemory;

			PortOut(IOPort::ModeRegister, *regPtr);
		}
		else
		{
			DMAModeRegister reg;
			u8* regPtr = (u8*)&reg;
			reg.channel = 2;
			reg.mode = TransferMode::Single;
			reg.reverse = false;
			reg.autoInit = false;
			reg.type = TransferType::MemoryToPeripheral;
		}
		Unmask(channel);
	}
}
