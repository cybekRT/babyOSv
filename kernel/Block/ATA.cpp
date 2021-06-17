#include"ATA.hpp"
#include"Block.hpp"
#include"HAL.h"
#include"Timer.h"
#include"Interrupt.h"

namespace ATA
{
	extern Block::BlockDriver drv;

	struct RegError
	{
		u8 addressMarkNotFound : 1;
		u8 trackZeroNotFound : 1;
		u8 aborted : 1;
		u8 mediaChangeRequest : 1;
		u8 idNotFound : 1;
		u8 mediaChanged : 1;
		u8 uncorrectableData : 1;
		u8 badBlockDetected : 1;
	};

	struct DriveHeadRegister
	{
		u8 lba_24_27 : 4;
		u8 driveNumber : 1;
		u8 reserved1 : 1;
		u8 isLBA : 1;
		u8 reserved2 : 1;
	};

	struct StatusRegister
	{
		u8 error : 1;
		u8 index : 1;
		u8 correctedData : 1;
		u8 drq : 1;
		u8 srv : 1;
		u8 driveFault : 1;
		u8 ready : 1;
		u8 busy : 1;

		void Print()
		{
			Terminal::Print("%c%c%c%c%c%c%c%c",
				(error ? 'E' : 'e'),
				(index ? 'I' : 'i'),
				(correctedData ? 'C' : 'c'),
				(drq ? 'D' : 'd'),
				(srv ? 'S' : 's'),
				(driveFault ? 'F' : 'f'),
				(ready ? 'R' : 'r'),
				(busy ? 'B' : 'b'));
		}
	};

	struct DeviceControlRegister
	{
		u8 reserved1 : 1;
		u8 disableInterrupts : 1;
		u8 reset : 1;
		u8 reserved2 : 4;
		u8 highOrderByte : 1;
	};

	struct DriveAddressRegister
	{
		u8 selectDrive0 : 1;
		u8 selectDrive1 : 1;
		u8 selectHead : 4;
		u8 writeGate : 1;
		u8 reserved : 1;
	};

	const u16 ioBase = 0x1F0;
	HAL::RegisterRW<u16> regData(ioBase + 0);
	HAL::RegisterRO<RegError> regError(ioBase + 1);
	HAL::RegisterWO<u8> regFeatures(ioBase + 1);

	HAL::RegisterRW<u8> regSectorCount(ioBase + 2);
	
	HAL::RegisterRW<u8> regLBALow(ioBase + 3);
	HAL::RegisterRW<u8> regLBAMid(ioBase + 4);
	HAL::RegisterRW<u8> regLBAHigh(ioBase + 5);

	HAL::RegisterRW<DriveHeadRegister> regDriveHead(ioBase + 6);
	HAL::RegisterRO<StatusRegister> regStatus(ioBase + 7);
	HAL::RegisterWO<u8> regCommand(ioBase + 7);

	const u16 ioBaseControl = 0x3F6;
	HAL::RegisterRO<StatusRegister> regAlternateStatus(ioBaseControl + 0);
	HAL::RegisterWO<DeviceControlRegister> regDeviceControl(ioBaseControl + 0);
	HAL::RegisterRO<DriveAddressRegister> regDriveAddress(ioBaseControl + 1);

	__attribute__((interrupt))
	void ISR_ATA(void*)
	{
		Print("ATA irq...\n");
		Interrupt::AckIRQ();
	}

	bool Read(u32 lba, void* dstBuffer)
	{
		while(regAlternateStatus.Read().busy);
		regDriveHead.Write( { .driveNumber = 0, .reserved1 = 1, .isLBA = 1, .reserved2 = 1 } );

		regSectorCount.Write(1);
		regLBALow.Write ( (lba >>  0) & 0xff );
		regLBAMid.Write ( (lba >>  8) & 0xff );
		regLBAHigh.Write( (lba >> 16) & 0xff );

		regCommand.Write(0x20); // ATA_CMD_READ_PIO

		while(!regAlternateStatus.Read().drq)
			Print(".");

		//regStatus.Read();

		Print("\n\nData:\n");

		//u8 buffer[512];
		u8* buffer = (u8*)dstBuffer;
		for(unsigned a = 0; a < 256; a++)
		{
			u16* ptr = (u16*)buffer;
			ptr[a] = regData.Read();

			Print("%X%X",	buffer[a * 2 + 0], 
							buffer[a * 2 + 1]);

			if(a % 16 == 15)
				Print("\n");
		}
	}

	bool Init()
	{
		Print("Partition entry size: %d\n", sizeof(MBRPartitionEntry));
		Print("MBR size: %d\n", sizeof(MBR));
		ASSERT(sizeof(MBR) == 512, "MBR has invalid size~!");

		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_ATA1), ISR_ATA);
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_ATA2), ISR_ATA);
		//Interrupt::Disable();

		regDeviceControl.Write( { .disableInterrupts = 0, .reset = 1 } );
		Timer::Delay(100);
		regDeviceControl.Write( { .disableInterrupts = 0, .reset = 0 } );
		Timer::Delay(100);

		regDriveHead.Write( { .driveNumber = 0, .reserved1 = 1, .reserved2 = 1 } );

		StatusRegister status;
		for(unsigned a = 0; a < 16; a++)
			status = regAlternateStatus.Read();

		Print("ATA status: ");
		status.Print();
		Print("\n");

		if(!status.ready)
		{
			Print("No drive...\n");
			return false;
		}

		auto cl = regLBAMid.Read();
		auto ch = regLBAHigh.Read();

		Print("ID: %x %x\n", cl, ch);

		//regFeatures.Write(0);

		__asm("xchg %bx, %bx");

		// Identify
		regDriveHead.Write( { .driveNumber = 0, .reserved1 = 1, .reserved2 = 1 } );
		regSectorCount.Write(0);
		regLBALow.Write(0);
		regLBAMid.Write(0);
		regLBAHigh.Write(0);

		regCommand.Write(0xEC);

		StatusRegister st;
		while((st = regStatus.Read()).busy)
			Print(".");

		st.Print();
		Print(" - Finished~!\n");

		Print("Status: ");
		regStatus.Read().Print();
		Print("\n");

		Print("Identify:\n");
		IDENTIFY_DEVICE_DATA idenData;
		u16* idenDataU16 = (u16*)&idenData;
		for(unsigned a = 0; a < 256; a++)
		{
			while(!regStatus.Read().drq);

			//u32 v = (regData.Read() << 16) | regData.Read();

			idenDataU16[a] = regData.Read();

			/*u8* tmptr = (u8*)&idenDataU16[a];

			u8 tmp = tmptr[0];
			tmptr[0] = tmptr[1];
			tmptr[1] = tmp;*/

			//Print("%c%c%c%c", v >> 24, v >> 16, v >> 8, v);
			//u32 v = (HAL::In16(0x1F0) << 16) | HAL::In16(0x1F0);
			//Print("%x ", v);

			//if(a % 16 == 7)
			//	Print("\n");
		}

		Print("Serial: %s\n", idenData.SerialNumber);
		Print("Name: %s\n", idenData.ModelNumber);
		Print("Cylinders: %d\n", idenData.NumCylinders);
		Print("Heads: %d\n", idenData.NumHeads);
		Print("Sectors: %d\n", idenData.NumSectorsPerTrack);
		Print("Max LBA48: %x%x\n", 	idenData.Max48BitLBA[0], 
									idenData.Max48BitLBA[1]);
		Print("LbaSupported: %d\n", idenData.Capabilities.LbaSupported);
		Print("CurrentSectorCapacity: %d\n", idenData.CurrentSectorCapacity);
		Print("UserAddressableSectors: %d\n", idenData.UserAddressableSectors);
		Print("ExtendedNumberOfUserAddressableSectors: %x%x\n", idenData.ExtendedNumberOfUserAddressableSectors[0],
																idenData.ExtendedNumberOfUserAddressableSectors[1]);
		Print("PhysicalLogicalSectorSize: %d\n", idenData.PhysicalLogicalSectorSize.LogicalSectorsPerPhysicalSector);

		Print("MAX_LBA24: %d\n", *(u32*)(((u8*)&idenData)+120) );
		Print("MAX_LBA48: %d\n", *(u32*)(((u8*)&idenData)+200) );

		Print("Status: ");
		regStatus.Read().Print();
		Print("\n");

		MBR mbr;
		Read(0, &mbr);

		for(unsigned a = 0; a < 4; a++)
		{
			auto& part = mbr.partitions[a];

			Print("=== Partition %d ===\n", a);
			Print("Boot: %d\n", part.bootable);
			Print("Type: %d\n", part.type);
			Print("LBA: %d - %d\n", part.lbaFirst, part.lbaSize);
			Print("Size: %d\n", part.lbaSize * 512 / 1024 / 1024);
		}

		/*for(unsigned a = 0; a < 512; a++)
		{

		}*/

		//Interrupt::Enable();

		return true;
	}
}