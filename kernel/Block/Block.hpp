#pragma once

#include"LinkedList.h"

namespace Block
{
	enum class Type
	{
		Unknown = 0,
		Floppy,
		HardDrive,
		CDRom,

		Count
	};

	struct BlockDriver
	{
		u8 (*Name)(void* dev, u8* buffer);
		u32 (*Size)(void* dev);

		u8 (*Lock)(void* dev);
		u8 (*Unlock)(void* dev);

		u32 (*BlockSize)(void* dev);
		u8 (*Read)(void* dev, u32 lba, u8* buffer);
		u8 (*Write)(void* dev, u32 lba, u8* buffer);
	};

	struct BlockDevice
	{
		Type			type;
		BlockDriver*	drv;
		void*			dev;
		u8				name[64];
	};

	extern LinkedList<BlockDevice*> devices;

	bool Init();

	void Register(Type type, BlockDriver* drv, void* dev);
	void Unregister(Type type, BlockDriver* drv, void* dev);
}
