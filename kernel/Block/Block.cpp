#include"Block.hpp"
#include"Containers/List.hpp"

//int strcpy(const char* src, char* dst);

#include"ATA.hpp"

namespace Block
{
	Array<BlockDevice*> devices;
	Array<BlockPartition*> partitions;

	uint8 devicesTypesCount[(unsigned)DeviceType::Count] = { 0 };
	const char* deviceTypeName[] = { "unk", "fdd", "hdd", "cd" };
	const char* partTypeName[] = { "r", "p", "e" };

	u8 BlockPartition::Read(u32 lba, u8* buffer)
	{
		if(lba >= lbaCount)
		{
			Print("Trying to read LBA %d/%d\n", lba, lbaCount);
			FAIL("Invalid LBA");
			return 1;
		}

		return device->drv->Read(device->drvPriv, deviceLbaOffset + lba, buffer);
	}

	u8 BlockPartition::Write(u32 lba, u8* buffer)
	{
		if(lba >= lbaCount)
		{
			Print("Trying to write LBA %d/%d\n", lba, lbaCount);
			FAIL("Invalid LBA");
			return 1;
		}

		return device->drv->Write(device->drvPriv, deviceLbaOffset + lba, buffer);
	}

	bool Init()
	{
		return true;
	}

	void ScanMBR(BlockDevice* dev)
	{
		ATA::MBR mbr;
		dev->drv->Read(dev->drvPriv, 0, (u8*)&mbr);

		Print("Scanning MBR...\n");
		for(auto& part : mbr.partitions)
		{
			auto devTotalLBA = dev->drv->Size(dev->drvPriv);
			if(part.type != 0 && part.reserved == 0 && part.lbaFirst < devTotalLBA && part.lbaSize < devTotalLBA)
			{
				RegisterPartition(Block::PartitionType::Primary, dev, nullptr, part.lbaFirst, part.lbaSize);
			}
		}
	}

	void RegisterDevice(DeviceType type, BlockDeviceDriver* drv, void* drvPriv)
	{
		BlockDevice* bd = new BlockDevice();// (BlockDevice*)Memory::Alloc(sizeof(BlockDevice));
		bd->type = type;
		bd->drv = drv;
		bd->drvPriv = drvPriv;

		Print("Type: %d, %p\n", type, &deviceTypeName[(unsigned)type]);
		Print("Type name: %s\n", deviceTypeName[(unsigned)type]);
		strcpy((char*)bd->name, deviceTypeName[(unsigned)type]);
		char tmp[2];
		tmp[0] = '0' + devicesTypesCount[(unsigned)type]++; tmp[1] = 0;
		strcat((char*)bd->name, tmp);

		Print("Registering device: %s\n", bd->name);
		devices.PushBack(bd);

		RegisterPartition(PartitionType::Raw, bd, drvPriv, 0, bd->drv->Size(drvPriv));
		ScanMBR(bd);
	}

	void RegisterPartition(PartitionType type, BlockDevice* dev, void* drvPriv, u32 lbaOffset, u32 lbaCount)
	{
		BlockPartition* part = new BlockPartition();// (BlockPartition*)Memory::Alloc(sizeof(BlockPartition));
		part->type = type;
		part->device = dev;
		part->deviceLbaOffset = lbaOffset;
		part->lbaCount = lbaCount;

		static int x = 0;
		auto nameLen = strcpy((char*)part->name, (char*)dev->name);
		Print("Dev: %s, Part: %s\n", dev->name, part->name);
		Print("Type: %d, len: %d\n", type, nameLen);
		strcat((char*)part->name, partTypeName[(u8)type]);
		Print("Name: %s, NameLen: %d\n", part->name, nameLen);

		char tmp[2];
		tmp[0] = '0' + x++; tmp[1] = 0;
		strcat((char*)part->name, tmp);
		//part->name[nameLen] = '0' + x++;
		//part->name[nameLen+1] = 0;

		Print("Registering partition: %s (%d %d)\n", part->name, part->deviceLbaOffset, part->lbaCount);
		partitions.PushBack(part);
	}

	Array<BlockDevice*> GetDevices()
	{
		return devices;
	}

	Array<BlockPartition*> GetPartitions()
	{
		return partitions;
	}
}
