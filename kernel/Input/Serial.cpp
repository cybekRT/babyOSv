#include"Serial.hpp"
#include"Containers/RingBuffer.hpp"
#include"Containers/Array.hpp"
#include"Interrupt.hpp"
#include"HAL.hpp"
#include"Timer.hpp"

using namespace Serial;

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

struct ModemStatusReg
{
	u8 ctsChanged	: 1; // 0 	Delta Clear to Send (DCTS) 	Indicates that CTS input has changed state since the last time it was read
	u8 dsrChanged	: 1; // 1 	Delta Data Set Ready (DDSR) 	Indicates that DSR input has changed state since the last time it was read
	u8 riChanged	: 1; // 2 	Trailing Edge of Ring Indicator (TERI) 	Indicates that RI input to the chip has changed from a low to a high state
	u8 dcdChanged	: 1; // 3 	Delta Data Carrier Detect (DDCD) 	Indicates that DCD input has changed state since the last time it ware read
	u8 cts			: 1; // 4 	Clear to Send (CTS) 	Inverted CTS Signal
	u8 dsr			: 1; // 5 	Data Set Ready (DSR) 	Inverted DSR Signal
	u8 ri			: 1; // 6 	Ring Indicator (RI) 	Inverted RI Signal
	u8 dcd			: 1; // 7 	Data Carrier Detect (DCD) 	Inverted DCD Signal
} __attribute__((packed));

enum class InterruptID
{
	// Priority from lowest to highest
	/* Interrupt Type | Interrupt Source | Interrupt Reset Control */
	none					= 0b0001,
	/**
	 * MODEM Status
	 * Clear to Send or Data Set Ready or Ring Indicator or Data Carrier Detect
	 * Reading the MODEM Status Register
	 */
	modemStatus				= 0b0000,
	/**
	 * Transmitter Holding Register Empty
	 * Transmitter Holding Register Empty
	 * Reading the IIR Register (if source of interrupt) or Writing into the Transmitter Holding Register
	 */
	transmitterHolding		= 0b0010,
	/**
	 * Character Timeout Indication
	 * No Characters Have Been Removed From or Input to the RCVR FIFO During the Last 4 Char.
	 * | Times and There Is at Least 1 Char. in It During This Time
	 * Reading the Receiver Buffer Register
	 */
	timeout					= 0b1100,
	/**
	 * Received Data Available
	 * Receiver Data Available or Trigger Level Reached
	 * Reading the Receiver Buffer Register or the FIFO Drops Below the Trigger Level
	 */
	receivedDataAvailable	= 0b0100,
	/**
	 * Receiver Line Status
	 * Overrun Error or Parity Error or Framing Error or Break Interrupt
	 * Reading the Line Status Register
	 */
	receiverLineStatus		= 0b0111,
};

struct InterruptIdentReg
{
	u8 interruptNotPending	: 1;
	InterruptID intId : 4;
	u8 fifoEnableRX	: 1;
	u8 fifoEnableTX	: 1;
} __attribute__((packed));

enum class FIFOTriggerLevel
{
	Bytes_1		= 0b00,
	Bytes_4		= 0b01,
	Bytes_8		= 0b10,
	Bytes_14	= 0b11,
};

/**
 * This is a write only register at the same location as the IIR
 * (the IIR is a read only register). This register is used to en-
 * able the FIFOs, clear the FIFOs, set the RCVR FIFO trigger
 * level, and select the type of DMA signalling
 */
struct FIFOControlReg
{
	/**
	 * Writing a 1 to FCR0 enables both the XMIT and RCVR
	 * FIFOs. Resetting FCR0 will clear all bytes in both FIFOs.
	 * When changing from the FIFO Mode to the 16450 Mode
	 * and vice versa, data is automatically cleared from the
	 * FIFOs. This bit must be a 1 when other FCR bits are written
	 * to or they will not be programmed.
	 */
	u8 enableTxRxFIFO				: 1;
	/**
	 * Writing a 1 to FCR1 clears all bytes in the RCVR FIFO
	 * and resets its counter logic to 0. The shift register is not
	 * cleared. The 1 that is written to this bit position is self-clear-
	 * ing.
	 */
	u8 clearRxFIFO					: 1;
	/**
	 * Writing a 1 to FCR2 clears all bytes in the XMIT FIFO
	 * and resets its counter logic to 0. The shift register is not
	 * cleared. The 1 that is written to this bit position is self-clear-
	 * ing.
	 */
	u8 clearTxFIFO					: 1;
	/**
	 * Setting FCR3 to a 1 will cause the RXRDY and
	 * TXRDY pins to change from mode 0 to mode 1 if FCR0e1
	 * (see description of RXRDY and TXRDY pins).
	 */
	u8 readyPinsMode				: 1;
	/**
	 * FCR4 to FCR5 are reserved for future use.
	 */
	u8 _reserved					: 2;
	/**
	 * FCR6 and FCR7 are used to set the trigger level for
	 * the RCVR FIFO interrupt.
	 */
	FIFOTriggerLevel triggerLevel	: 2;

} __attribute__((packed));

struct LineControlReg
{
	WordLength wordLength	: 2;
	StopBits stopBits		: 1;
	Parity parity			: 3;
	u8 setBreak				: 1;
	u8 dlab					: 1;
} __attribute__((packed));

namespace Serial
{
	RingBuffer<u8, 8> comBuffersIn[4];
	RingBuffer<u8, 16> comBuffersOut[4];
	bool comWorking[4];
	Array<Handler> handlers[4];

	ISR(COM_1_3)
	{
		Print("%s\n", __FUNCTION__);
		Interrupt::AckIRQ();
	}

	ISR(COM_2_4)
	{
		Print("%s\n", __FUNCTION__);
		Interrupt::AckIRQ();
	}

	bool Probe(int port)
	{
		u16 base = portIO[port];

		u8 testVal = 0xA5;
		HAL::Out8(base + IO_REG::Data, testVal);

		HAL::RegisterRO<LineStatusReg> lineStatus(base + IO_REG::LineStatus);

		WAIT_UNTIL(lineStatus.Read().inputReady, 1000);

		u8 recv = HAL::In8(base);

		return (recv == testVal);
	}

	bool SetBaudRate(u8 port, u32 baudRate)
	{
		ASSERT(port < 4, "Invalid COM port");
		u16 base = portIO[port];

		HAL::RegisterRW<LineControlReg> lcReg(base + IO_REG::LineControl);
		lcReg.Read();

		// Enable DLAB
		lcReg.value.dlab = 1;
		lcReg.Write();

		// Calculate
		ASSERT((115200 % baudRate) == 0, "Invalid baudrate~!");
		u32 divisor = 115200 / baudRate;

		// Send
		HAL::Out8(base + IO_REG::BaudLow, divisor & 0xff);
		HAL::Out8(base + IO_REG::BaudHigh, (divisor >> 8) & 0xff);

		// Disable DLAB
		lcReg.value.dlab = 0;
		lcReg.Write();

		return true;
	}

	bool Init()
	{
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_COM1), ISR_COM_1_3);
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_COM2), ISR_COM_2_4);

		for(unsigned port = 0; port < 4; port++)
		{
			u16 base = portIO[port];

			HAL::RegisterRW<InterruptEnableReg> ieReg(base + IO_REG::InterruptEnable);
			ieReg.value.OnError = 0;
			ieReg.value.OnInputEmpty = 0;
			ieReg.value.OnOutputFull = 0;
			ieReg.value.OnStatusChange = 0;
			ieReg.Write();

			SetBaudRate(port, 115200);

			HAL::RegisterRW<LineControlReg> lineCtrlReg(base + IO_REG::LineControl);
			lineCtrlReg.Read();
			lineCtrlReg.value.dlab = 0;
			lineCtrlReg.value.parity = Parity::None;
			lineCtrlReg.value.stopBits = StopBits::Bits_1;
			lineCtrlReg.value.wordLength = WordLength::Bits_8;
			lineCtrlReg.Write();

			HAL::RegisterRW<ModemControlReg> modemReg(base + IO_REG::ModemControl);
			modemReg.Read();
			modemReg.value.enableIRQ = false;
			modemReg.value.loopback = true;
			modemReg.value.dataTerminalReady = false;
			modemReg.value.requestToSend = false;
			modemReg.Write();

			// FIXME: COM0 is sometimes not detected on Qemu
			for(unsigned a = 0; a < 5; a++)
			{
				comWorking[port] = Probe(port);
				if(comWorking[port])
					break;
				else
					Timer::Delay(20);
			}

			Print("COM%d status: %d\n", port, comWorking[port]);

			// If port is not working, ignore it and jump to next one
			if(!comWorking[port])
				continue;

			// Set FIFOs
			HAL::RegisterRW<FIFOControlReg> fifoReg(base + IO_REG::FIFOControl);
			fifoReg.Read();
			fifoReg.value.clearRxFIFO = true;
			fifoReg.value.clearTxFIFO = true;
			fifoReg.value.enableTxRxFIFO = true;
			fifoReg.value.readyPinsMode = 0;
			fifoReg.value.triggerLevel = FIFOTriggerLevel::Bytes_14;
			fifoReg.Write();

			ieReg.value.OnError = true;
			ieReg.value.OnInputEmpty = true;
			ieReg.value.OnOutputFull = true;
			ieReg.value.OnStatusChange = true;
			ieReg.Write();

			modemReg.value.enableIRQ = true;
			modemReg.value.loopback = false;
			modemReg.Write();
		}

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

	bool Configure(u8 port, u32 baudRate, WordLength wordLength, StopBits stopBits, Parity parity, bool breakLine)
	{
		SetBaudRate(port, 115200);

		HAL::RegisterRW<LineControlReg> lineCtrlReg(portIO[port] + IO_REG::LineControl);
		lineCtrlReg.Read();
		lineCtrlReg.value.dlab = 0;
		lineCtrlReg.value.parity = parity;
		lineCtrlReg.value.stopBits = stopBits;
		lineCtrlReg.value.setBreak = breakLine;
		lineCtrlReg.value.wordLength = wordLength;
		lineCtrlReg.Write();

		return true;
	}

	bool SetReady(u8 port, bool rts, bool dtr)
	{
		HAL::RegisterRW<ModemControlReg> modemReg(portIO[port] + IO_REG::ModemControl);
		modemReg.Read();
		modemReg.value.dataTerminalReady = dtr;
		modemReg.value.requestToSend = rts;
		modemReg.Write();

		return true;
	}

	void ClearBuffers(u8 port)
	{
		HAL::RegisterRW<FIFOControlReg> fifoReg(portIO[port] + IO_REG::FIFOControl);
		fifoReg.Read();
		fifoReg.value.clearRxFIFO = true;
		fifoReg.value.clearTxFIFO = true;
		fifoReg.Write();
	}

	void Test(u8 port)
	{
		Print("Test: ");
		HAL::RegisterRO<LineStatusReg> statusReg(portIO[port] + IO_REG::LineStatus);
		while(statusReg.Read().outputFull)
		{

		}
	}

	bool ReadByte(u8 port, u8* value, u32 timeout)
	{
		HAL::RegisterRO<LineStatusReg> statusReg(portIO[port] + IO_REG::LineStatus);
		if(!WAIT_UNTIL(statusReg.Read().outputFull, timeout))
			return false;

		*value = HAL::In8(portIO[port] + IO_REG::Data);
		return true;
	}

	bool WriteByte(u8 port, u8 value, u32 timeout)
	{
		HAL::RegisterRO<LineStatusReg> statusReg(portIO[port] + IO_REG::LineStatus);
		if(!WAIT_UNTIL(statusReg.Read().inputReady, timeout))
			return false;

		HAL::Out8(portIO[port] + IO_REG::Data, value);
		return true;
	}
};
