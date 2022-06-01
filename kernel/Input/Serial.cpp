#include"Serial.hpp"
#include"Containers/RingBuffer.hpp"
#include"Containers/Array.hpp"
#include"Interrupt.hpp"

/* Ports are counted from 0, COMx are from 1 :| */
#define	IO_COM1 0x3F8
#define	IO_COM2 0x2F8
#define	IO_COM3 0x3E8
#define	IO_COM4 0x2E8
#define	IO_COM5 0x5F8
#define	IO_COM6 0x4F8
#define	IO_COM7 0x5E8
#define	IO_COM8 0x4E8

const u16 portIO[4] = {
	0x3F8,
	0x2F8,
	0x3E8,
	0x2E8,
};

enum IO_REG {
	Data 				= 0, // 0 	Data register. Reading this registers read from the Receive buffer. Writing to this register writes to the Transmit buffer.
	InterruptEnable 	= 1, // 0 	Interrupt Enable Register.
	BaudLow 			= 0, // 1 	With DLAB set to 1, this is the least significant byte of the divisor value for setting the baud rate.
	BaudHigh 			= 1, // 1 	With DLAB set to 1, this is the most significant byte of the divisor value.
	FIFOControl 		= 2, // - 	Interrupt Identification and FIFO control registers
	LineControl 		= 3, // - 	Line Control Register. The most significant bit of this register is the DLAB.
	ModemControl 		= 4, // - 	Modem Control Register.
	LineStatus 			= 5, // - 	Line Status Register.
	ModemStatus 		= 6, // - 	Modem Status Register.
	Scratch 			= 7, // - 	Scratch Register.
};

struct InterruptEnableReg
{
	u8 OnOutputFull 	: 1; // 0 	Data available
	u8 OnInputEmpty 	: 1; // 1 	Transmitter empty
	u8 OnError 			: 1; // 2 	Break/error
	u8 OnStatusChange 	: 1; // 3 	Status change
	u8 unused 			: 4; // 4-7	Unused
} __attribute__((packed));

struct ModemControlReg
{
	u8 dataTerminalReady	: 1; // 0 	Data Terminal Ready (DTR) 	Controls the Data Terminal Ready Pin
	u8 requestToSend		: 1; // 1 	Request to Send (RTS) 	Controls the Request to Send Pin
	u8 _unused1				: 1; // 2 	Out 1 	Controls a hardware pin (OUT1) which is unused in PC implementations
	u8 enableIRQ			: 1; // 3 	Out 2 	Controls a hardware pin (OUT2) which is used to enable the IRQ in PC implementations
	u8 loopback				: 1; // 4 	Loop 	Provides a local loopback feature for diagnostic testing of the UART
	u8 _unused2				: 3; // 5-7 Unused
} __attribute__((packed));

struct LineStatusReg
{
	u8 outputFull 		: 1; // 0 	Data ready (DR) 	Set if there is data that can be read
	u8 errorOverrun 	: 1; // 1 	Overrun error (OE) 	Set if there has been data lost
	u8 errorParity 		: 1; // 2 	Parity error (PE) 	Set if there was an error in the transmission as detected by parity
	u8 errorFraming 	: 1; // 3 	Framing error (FE) 	Set if a stop bit was missing
	u8 breakIndicator 	: 1; // 4 	Break indicator (BI) 	Set if there is a break in data input
	u8 inputReady 		: 1; // 5 	Transmitter holding register empty (THRE) 	Set if the transmission buffer is empty (i.e. data can be sent)
	u8 inputEmpty 		: 1; // 6 	Transmitter empty (TEMT) 	Set if the transmitter is not doing anything
	u8 errorImpending 	: 1; // 7 	Impending Error 	Set if there is an error with a word in the input buffer
} __attribute__((packed));

namespace Serial
{
	RingBuffer<u8, 8> comBuffersIn[4];
	RingBuffer<u8, 16> comBuffersOut[4];
	Array<Handler> handlers[4];

	ISR(COM_1_3)
	{
		Print("%s\n", __FUNCTION__);
	}

	ISR(COM_2_4)
	{
		Print("%s\n", __FUNCTION__);
	}

	bool Init()
	{
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_COM1), ISR_COM_1_3);
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_COM2), ISR_COM_2_4);

		return true;
	}

	void Register(u8 port, Handler handler)
	{
		ASSERT(port < 4, "Invalid COM port");
		handlers[port].PushBack(handler);
	}

	void Unregister(u8 port, Handler handler)
	{
		ASSERT(port < 4, "Invalid COM port");
		handlers[port].Remove(handler);
	}
};
