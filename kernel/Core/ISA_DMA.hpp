#include"HAL.h"

namespace ISA_DMA
{
	enum class TransferDir
	{
		READ,
		WRITE
	};

	enum class IOPort
	{
		StartAddressChannel0 				=	0x00,	//	0xC0	Word	W
		CountRegisterChannel0				=	0x01,	//	0xC2	Word	W
		StartAddressRegisterChannel1		=	0x02,	//	0xC4	Word	W
		CountRegisterChannel1				=	0x03,	//	0xC6	Word	W
		StartAddressRegisterChannel2		=	0x04,	//	0xC8	Word	W
		CountRegisterChannel2				=	0x05,	//	0xCA	Word	W
		StartAddressRegisterChannel3		=	0x06,	//	0xCC	Word	W
		CountRegisterChannel3				=	0x07,	//	0xCE	Word	W
		StatusRegister						=	0x08,	//	0xD0	Byte	R
		CommandRegister						=	0x08,	//	0xD0	Byte	W
		RequestRegister						=	0x09,	//	0xD2	Byte	W
		SingleChannelMaskRegister			=	0x0A,	//	0xD4	Byte	W
		ModeRegister						=	0x0B,	//	0xD6	Byte	W
		FlipFlopResetRegister				=	0x0C,	//	0xD8	Byte	W
		IntermediateRegister				=	0x0D,	//	0xDA	Byte	R
		MasterResetRegister					=	0x0D,	//	0xDA	Byte	W
		MaskResetRegister					=	0x0E,	//	0xDC	Byte	W
		MultiChannelMaskRegister			=	0x0F,	//	0xDE	Byte	R

		PageAddressRegisterChannel0			=	0x87, // (unusable)
		PageAddressRegisterChannel1			=	0x83,
		PageAddressRegisterChannel2			=	0x81,
		PageAddressRegisterChannel3			=	0x82,
		PageAddressRegisterChannel4			=	0x8F, // (unusable)
		PageAddressRegisterChannel5			=	0x8B,
		PageAddressRegisterChannel6			=	0x89,
		PageAddressRegisterChannel7 		=	0x8A
	};

	enum class TransferType : u8
	{
		SelfTest			=	0b00,
		PeripheralToMemory	=	0b01,
		MemoryToPeripheral	=	0b10,
		Invalid				=	0b11
	};

	enum class TransferMode : u8
	{
		OnDemand		=	0b00,
		Single			=	0b01,
		Block			=	0b10,
		Cascade			=	0b11
	};

	struct DMAModeRegister
	{
		u8 channel : 2;
		TransferType type : 2;
		u8 autoInit : 1;
		u8 reverse : 1;
		TransferMode mode : 2;
	} __attribute__((packed));

	bool Init();
	void Start(u8 channel, TransferDir dir, void* physAddress, u16 count);
}