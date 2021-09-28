#pragma once

#include"Container/Array.h"

namespace Block
{
	enum class DeviceType
	{
		Unknown = 0,
		Floppy,
		HardDrive,
		CDRom,

		Count
	};

	enum class PartitionType
	{
		Raw = 0,
		Primary = 1,
		Extended = 2,

		Count
	};

	struct BlockDeviceDriver
	{
		u8 (*Name)(void* dev, u8* buffer);

		/* Number of sectors */
		u32 (*Size)(void* dev);
		u32 (*BlockSize)(void* dev);

		u8 (*Lock)(void* dev);
		u8 (*Unlock)(void* dev);

		u8 (*Read)(void* dev, u32 lba, u8* buffer);
		u8 (*Write)(void* dev, u32 lba, u8* buffer);
	};

	struct BlockDevice
	{
		DeviceType			type;
		BlockDeviceDriver*	drv;
		void*				drvPriv;
		u8					name[64];
	};

	struct BlockPartition
	{
		PartitionType	type;
		BlockDevice*	device;
		u32				lbaOffset;
		u32				lbaCount;
		u8				name[64];

		u8 Read(u32 lba, u8* buffer);
	};

	//extern Array<BlockDevice> devices;
	//extern Array<BlockPartition> partitions;

	bool Init();

	void RegisterDevice(DeviceType type, BlockDeviceDriver* drv, void* drvPriv);
	void RegisterPartition(PartitionType type, BlockDevice* dev, void* drvPriv, u32 lbaOffset, u32 lbaCount);

	Array<BlockDevice*> GetDevices();
	Array<BlockPartition*> GetPartitions();
}
