#include"Floppy.hpp"
#include"Memory.h"
#include"ISA_DMA.hpp"
#include"Interrupt.h"
#include"Timer.h"
#include"Block.hpp"
#include"Thread.hpp"
#include<stdarg.h>

namespace Floppy
{
	enum class IOPort
	{
		FDD_REG_STATUS_A	= 0x3F0, // RO
		FDD_REG_STATUS_B	= 0x3F1, // RO
		FDD_REG_DIGITAL_OUT	= 0x3F2,
		FDD_REG_TAPE_DRIVE	= 0x3F3,
		FDD_REG_MAIN_STATUS	= 0x3F4, // RO
		FDD_REG_DATARATE_SELECT	= 0x3F4, // WO
		FDD_REG_DATA_FIFO	= 0x3F5,
		FDD_REG_DIGITAL_IN	= 0x3F7, // RO
		FDD_REG_CONF_CONTROL	= 0x3F7, // WO
	};

	enum class DOR_DSel
	{
		FDD_DOR_DSELA		= (0b00 << 0),
		FDD_DOR_DSELB		= (0b01 << 0),
		FDD_DOR_DSELC		= (0b10 << 0),
		FDD_DOR_DSELD		= (0b11 << 0),
	};

	/*FDD_DOR_RESET		equ (1 << 2)
	FDD_DOR_IRQ			equ (1 << 3)
	FDD_DOR_MOTA		equ (1 << 4)
	FDD_DOR_MOTB		equ (1 << 5)
	FDD_DOR_MOTC		equ (1 << 6)
	FDD_DOR_MOTD		equ (1 << 7)*/

	struct DigitalOutputRegister
	{
		DOR_DSel driveSelect : 2;
		u8 reset : 1;
		u8 irq : 1;
		u8 motorA : 1;
		//u8 motorB : 1;
		//u8 motorC : 1;
		//u8 motorD : 1;
		u8 unused : 3;
	} __attribute__((packed));

	struct MainStatusRegister
	{
		u8 active : 1;
		u8 unused : 3;
		u8 commandBusy : 1;
		u8 ndma : 1;
		u8 dio : 1;
		u8 rqm : 1;
	} __attribute__((packed));

	/*FDD_MSR_ACTA		equ (1 << 0) // Drive 0 is seeking
	FDD_MSR_ACTB		equ (1 << 1) // Drive 1 is seeking
	FDD_MSR_ACTC		equ (1 << 2) // Drive 2 is seeking
	FDD_MSR_ACTD		equ (1 << 3) // Drive 3 is seeking
	FDD_MSR_CB		equ (1 << 4) // Command Busy: set when command byte received, cleared at end of Result phase
	FDD_MSR_NDMA		equ (1 << 5) // Set in Execution phase of PIO mode read/write commands only
	FDD_MSR_DIO		equ (1 << 6) // Set if FIFO IO port expects an IN opcode
	FDD_MSR_RQM		equ (1 << 7) // Set if it's OK (or mandatory) to exchange bytes with the FIFO IO port*/

	enum class DataRateSelectType
	{
		FDD_DSR_TYPE_144	= (0b00 << 0), // 1.44MB
		FDD_DSR_TYPE_288	= (0b11 << 0), // 2.88MB
	};

	struct DigitalInputRegister
	{
		u8 unused : 7;
		u8 mediaChanged : 1;
	} __attribute__((packed));

	//FDD_DIR_MEDIA_CHANGED	equ (1 << 7) // media changed

	enum Command
	{
		FDD_CMD_READ_TRACK =                 2,	// generates IRQ6
		FDD_CMD_SPECIFY =                    3,      // * set drive parameters
		FDD_CMD_SENSE_DRIVE_STATUS =         4,
		FDD_CMD_WRITE_DATA =                 5,      // * write to the disk
		FDD_CMD_READ_DATA =                  6,      // * read from the disk
		FDD_CMD_RECALIBRATE =                7,      // * seek to cylinder 0
		FDD_CMD_SENSE_INTERRUPT =            8,      // * ack IRQ6; get status of last command
		FDD_CMD_WRITE_DELETED_DATA =         9,
		FDD_CMD_READ_ID =                    10,	// generates IRQ6
		FDD_CMD_READ_DELETED_DATA =          12,
		FDD_CMD_FORMAT_TRACK =               13,     // *
		FDD_CMD_DUMPREG =                    14,
		FDD_CMD_SEEK =                       15,     // * seek both heads to cylinder X
		FDD_CMD_VERSION =                    16,	// * used during initialization; once
		FDD_CMD_SCAN_EQUAL =                 17,
		FDD_CMD_PERPENDICULAR_MODE =         18,	// * used during initialization; once; maybe
		FDD_CMD_CONFIGURE =                  19,     // * set controller parameters
		FDD_CMD_LOCK =                       20,     // * protect controller params from a reset
		FDD_CMD_VERIFY =                     22,
		FDD_CMD_SCAN_LOW_OR_EQUAL =          25,
		FDD_CMD_SCAN_HIGH_OR_EQUAL =         29,

		FDD_CMD_OPTION_MULTITRACK		= 0x80,
		FDD_CMD_OPTION_MFM			= 0x40,
		FDD_CMD_OPTION_SKIP			= 0x20
	};

	extern Block::BlockDriver drv;
	void* dmaPhys;
	void* dmaLogic;
	volatile u8 irqReceived = 0;

	u8 currentTrack = 0;
	u8 motorEnabled = 0;

	__attribute__((interrupt))
	void ISR_Floppy(void*)
	{
		irqReceived = 1;
		Thread::RaiseSignal(Thread::Signal { .type = Thread::Signal::Type::IRQ, .addr = Interrupt::IRQ_FLOPPY }, 0);
		Interrupt::AckIRQ();
	}

	u8 PortIn(IOPort port)
	{
		return HAL::In((u16)port);
	}

	void PortOut(IOPort port, u8 data)
	{
		HAL::Out((u16)port, data);
	}

	MainStatusRegister ReadMainStatusRegister()
	{
		MainStatusRegister reg;
		u8 *regPtr = (u8*)&reg;
		*regPtr = PortIn(IOPort::FDD_REG_MAIN_STATUS);

		return reg;
	}

	void WaitForOut()
	{
		auto t = Timer::GetTicks();
		for(;;)
		{
			auto reg = ReadMainStatusRegister();
			if(reg.rqm && !reg.dio)
				break;

			Thread::NextThread();

			if(Timer::GetTicks() >= t + 200)
				break;
		}
	}

	void WaitForIn()
	{
		auto t = Timer::GetTicks();
		for(;;)
		{
			auto reg = ReadMainStatusRegister();
			if(reg.rqm && reg.dio)
				break;

			Thread::NextThread();

			if(Timer::GetTicks() >= t + 200)
				break;
		}
	}

	void WaitIRQ()
	{
		Thread::WaitForSignal(Thread::Signal { .type = Thread::Signal::Type::IRQ, .addr = Interrupt::IRQ_FLOPPY }, -1);
	}

	void MotorOn()
	{
		if(motorEnabled++ > 0)
			return;

		DigitalOutputRegister reg;
		u8* regPtr = (u8*)&reg;

		reg.driveSelect = DOR_DSel::FDD_DOR_DSELA;
		reg.reset = 1;
		reg.irq = 1;
		reg.motorA = 1;

		PortOut(IOPort::FDD_REG_DIGITAL_OUT, *regPtr);
		Timer::Delay(300);
	}

	void MotorOff()
	{
		ASSERT(motorEnabled > 0, "Motor is disabled");
		if(--motorEnabled > 0)
			return;

		DigitalOutputRegister reg;
		u8* regPtr = (u8*)&reg;

		reg.driveSelect = DOR_DSel::FDD_DOR_DSELA;
		reg.reset = 1;
		reg.irq = 1;
		reg.motorA = 0;

		PortOut(IOPort::FDD_REG_DIGITAL_OUT, *regPtr);
	}

	void WriteData(IOPort port, u8 data)
	{
		WaitForOut();
		PortOut(port, data);
	}

	void WriteData(u8 data)
	{
		WriteData(IOPort::FDD_REG_DATA_FIFO, data);
	}

	u8 ReadData(IOPort port)
	{
		WaitForIn();
		return PortIn(port);
	}

	u8 ReadData()
	{
		WaitForIn();
		return PortIn(IOPort::FDD_REG_DATA_FIFO);
	}

	void Exec(Command cmd, unsigned paramsCount = 0, ...)
	{
		va_list args;
		va_start(args, paramsCount);

		u8 data[16];
		data[0] = (u8)cmd;

		for(unsigned a = 0; a < paramsCount; a++)
		{
			u8 param = va_arg(args, u32);
			data[1 + a] = param;
		}

		va_end(args);

		for(unsigned a = 0; a < 1 + paramsCount; a++)
		{
			WriteData(data[a]);
		}
	}

	void Reset()
	{
		DigitalOutputRegister reg;
		u8* regPtr = (u8*)&reg;

		// Reset
		reg.driveSelect = DOR_DSel::FDD_DOR_DSELA;
		reg.reset = 0;
		reg.irq = 1;
		reg.motorA = 0;

		PortOut(IOPort::FDD_REG_DIGITAL_OUT, *regPtr);

		Timer::Delay(100);

		// Select data rate
		u8 dataRate = (u8)DataRateSelectType::FDD_DSR_TYPE_144;
		PortOut(IOPort::FDD_REG_CONF_CONTROL, dataRate);

		// Un-reset
		reg.driveSelect = DOR_DSel::FDD_DOR_DSELA;
		reg.reset = 1;
		reg.irq = 1;
		reg.motorA = 0;

		//irqReceived = 0;
		Thread::SetState(nullptr, Thread::State::Unstoppable);
		PortOut(IOPort::FDD_REG_DIGITAL_OUT, *regPtr);
		WaitIRQ();

		for(unsigned a = 0; a < 4; a++)
		{
			Exec(Command::FDD_CMD_SENSE_INTERRUPT);
			ReadData();
			ReadData();
		}

		Exec(Command::FDD_CMD_SPECIFY, 2, 0xDF, 0x02);
	}

	void Recalibrate()
	{
		MotorOn();

		Thread::SetState(nullptr, Thread::State::Unstoppable);
		Exec(Command::FDD_CMD_RECALIBRATE, 1, 0x00);
		WaitIRQ();

		Exec(Command::FDD_CMD_SENSE_INTERRUPT);
		u8 status = ReadData();
		ReadData();

		if(!(status & (1 << 5)))
			FAIL("Floppy");
		if(status & (1 << 4))
			FAIL("Floppy");

		MotorOff();
	}

	void Lock()
	{
		Exec(Command::FDD_CMD_LOCK);
		ReadData();
	}

	bool Init()
	{
		Print("Initializing floppy...\n");

		dmaPhys = Memory::AllocPhys(4096);
		dmaLogic = Memory::Map(dmaPhys, nullptr, 4096);

		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_FLOPPY), ISR_Floppy);

		Reset();
		Recalibrate();
		Lock();

		Block::Register(Block::Type::Floppy, &drv, nullptr);

		Print("Initialized floppy...\n");
		return true;
	}

	void Seek(u8 track)
	{
		if(currentTrack == track)
		{
			return;
		}

		Print("Seeking track: %u\n", track);

		Thread::SetState(nullptr, Thread::State::Unstoppable);
		Exec(Command::FDD_CMD_SEEK, 2, 0x00, track);
		WaitIRQ();

		Exec(Command::FDD_CMD_SENSE_INTERRUPT);
		u8 status = ReadData();
		ReadData();

		MainStatusRegister* reg = (MainStatusRegister*)&status;
		if(!reg->ndma || reg->rqm)
			FAIL("floppy seek");

		currentTrack = track;
	}

	u8 Read(void* dev, u32 lba, u8* buffer)
	{
		MotorOn();

		u8 cylinder = lba / 18 / 2;
		u8 head = (lba / 18) % 2;
		u8 sector = lba % 18 + 1;

		Seek(cylinder);

		ISA_DMA::Start(2, ISA_DMA::TransferDir::READ, dmaPhys, 512);

		irqReceived = 0;
		u8 cmd = (u8)FDD_CMD_OPTION_MULTITRACK | (u8)FDD_CMD_OPTION_MFM | (u8)FDD_CMD_OPTION_SKIP | (u8)FDD_CMD_READ_DATA;
		u8 driveNo = 0;

		Thread::SetState(nullptr, Thread::State::Unstoppable);
		Exec((Command)cmd, 8, (head << 2) | driveNo, cylinder, head, sector, 0x02, 0x12, 0x1B, 0xFF);
		WaitIRQ();

		u8 st0 = ReadData();
		u8 st1 = ReadData();
		u8 st2 = ReadData();
		u8 stc = ReadData();
		u8 sth = ReadData();
		u8 str = ReadData();
		u8 stn = ReadData();

		if(st0 & 0b11000000 || stn != 0x02)
		{
			static int retryCounter = 0;
			// PCem needs this, uh...
			// TODO: decide if FAIL is appropriate or
			// maybe always reset and return with failure...

			if(++retryCounter > 10)
				FAIL("floppy read status - 10 fails");

			Print("Retry... ");
			MotorOff();
			Reset();
			
			//return Read(dev, lba, buffer);
			return 1;
		}

		MotorOff();

		u8* src = (u8*)dmaLogic;
		u8* dst = (u8*)buffer;
		for(unsigned a = 0; a < 512; a++)
		{
			*dst++ = *src++;
		}

		return 0;
	}

	u8 Name(void* dev, u8* buffer)
	{
		u8* name = (u8*)"Floppy";
		for(unsigned a = 0; ; a++)
		{
			buffer[a] = name[a];

			if(!name[a])
				return a;
		}

		return 0;
	}

	u32 Size(void* dev)
	{
		return 2880;
	}

	u8 Lock(void* dev)
	{
		MotorOn();
		return 0;
	}

	u8 Unlock(void* dev)
	{
		MotorOff();
		return 0;
	}

	u32 BlockSize(void* dev)
	{
		return 512;
	}

	u8 _Write(void* dev, u32 lba, u8* buffer)
	{
		return 1;
	}

	Block::BlockDriver drv
	{
		.Name = Name,
		.Size = Size,

		.Lock = Lock,
		.Unlock = Unlock,

		.BlockSize = BlockSize,
		.Read = Read,
		.Write = _Write
	};
}