#include"Floppy.hpp"
#include"Memory.h"
#include"ISA_DMA.hpp"
#include"Interrupt.h"
#include"Timer.h"
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

	void* dmaPhys;
	void* dmaLogic;
	volatile u8 irqReceived = 0;

	__attribute__((interrupt))
	void ISR_Floppy(void*)
	{
		irqReceived = 1;

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
		//Print("FDD wait for out\n");
		for(;;)
		{
			auto reg = ReadMainStatusRegister();
			if(reg.rqm && !reg.dio)
				break;

			HALT;
		}
	}

	void WaitForIn()
	{
		//Print("FDD wait for in\n");
		for(;;)
		{
			auto reg = ReadMainStatusRegister();
			if(reg.rqm && reg.dio)
				break;

			HALT;
		}
	}

	void WaitIRQ()
	{
		while(!irqReceived)
		{
			HALT;
		}
	}

	void MotorOn()
	{
		Print("FDD motor on\n");

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
		Print("FDD motor off\n");

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

		Print("Executing command: %x", data[0]);

		//WriteData(cmd);
		for(unsigned a = 0; a < paramsCount; a++)
		{
			u8 param = va_arg(args, u32);
			Print(", %x", param);

			data[1 + a] = param;
			//WriteData(param);
		}

		Print("\n");
		va_end(args);

		for(unsigned a = 0; a < 1 + paramsCount; a++)
		{
			WriteData(data[a]);
		}
	}

	void Reset()
	{
		Print("FDD reset\n");
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

		//Timer::Delay(1000);

		// Un-reset
		reg.driveSelect = DOR_DSel::FDD_DOR_DSELA;
		reg.reset = 1;
		reg.irq = 1;
		reg.motorA = 0;

		irqReceived = 0;

		PortOut(IOPort::FDD_REG_DIGITAL_OUT, *regPtr);

		//while(!irqReceived);
		WaitIRQ();

		for(unsigned a = 0; a < 4; a++)
		{
			//Print("Sensing...\n");
			//WriteData(Command::FDD_CMD_SENSE_INTERRUPT);
			Exec(Command::FDD_CMD_SENSE_INTERRUPT);
			ReadData();
			ReadData();
		}

		Exec(Command::FDD_CMD_SPECIFY, 2, 0xDF, 0x02);
	}

	void Recalibrate()
	{
		Print("FDD recalibrate\n");

		MotorOn();

		irqReceived = 0;
		Exec(Command::FDD_CMD_RECALIBRATE, 1, 0x00);
		//while(!irqReceived);
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
		Print("FDD lock\n");

		Exec(Command::FDD_CMD_LOCK);
		ReadData();
	}

	bool Init()
	{
		Print("Initializing floppy...\n");

		dmaPhys = Memory::AllocPhys(4096);
		dmaLogic = Memory::Map(dmaPhys, nullptr, 4096);
		Print("Floppy DMA: %p, %p\n", dmaPhys, dmaLogic);

		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_FLOPPY), ISR_Floppy);

		Reset();
		Recalibrate();
		Lock();

		static u8 tmpBuffer[512];
		Read(0, tmpBuffer);

		//Print("Tmp: %x %x\n", tmpBuffer[510], tmpBuffer[511]);

		if(tmpBuffer[510] != 0x55 || tmpBuffer[511] != 0xAA)
			FAIL("reading floppy");

		Print("Initialized floppy...\n");
		return true;
	}

	void Seek(u8 track)
	{
		Print("Seeking track: %u\n", track);

		irqReceived = 0;
		Exec(Command::FDD_CMD_SEEK, 2, 0x00, track);
		//while(!irqReceived);
		WaitIRQ();

		//Print("Sensing...\n");
		Exec(Command::FDD_CMD_SENSE_INTERRUPT);
		u8 status = ReadData();
		ReadData();

		MainStatusRegister* reg = (MainStatusRegister*)&status;
		if(!reg->ndma || reg->rqm)
			FAIL("floppy seek");
	}

	void Read(u16 lba, void* buffer)
	{
		MotorOn();

		u8 cylinder = lba / 18 / 2;
		u8 head = (lba / 18) % 2;
		u8 sector = lba % 18 + 1;

		//Print("LBA %x -> CHS %x %x %x\n", lba, cylinder, head, sector);

		Seek(cylinder);

		ISA_DMA::Start(2, ISA_DMA::TransferDir::READ, dmaPhys, 512);

		irqReceived = 0;
		u8 cmd = (u8)FDD_CMD_OPTION_MULTITRACK | (u8)FDD_CMD_OPTION_MFM | (u8)FDD_CMD_OPTION_SKIP | (u8)FDD_CMD_READ_DATA;
		u8 driveNo = 0;
		Exec((Command)cmd, 8, driveNo, cylinder, head, sector, 0x02, 0x12, 0x1B, 0xFF);

		WaitIRQ();

		u8 st0 = ReadData();
		u8 st1 = ReadData();
		u8 st2 = ReadData();
		u8 stc = ReadData();
		u8 sth = ReadData();
		u8 str = ReadData();
		u8 stn = ReadData();

		if(st0 & 0b11000000)
			FAIL("floppy read status");

		//Print("Floppy result: %x %x %x %x %x %x %x\n", st0, st1, st2, stc, sth, str, stn);

		MotorOff();

		u8* src = (u8*)dmaLogic;
		u8* dst = (u8*)buffer;
		for(unsigned a = 0; a < 512; a++)
		{
			*dst++ = *src++;
		}
	}
}