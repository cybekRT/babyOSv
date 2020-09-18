#pragma once

#include"LinkedList.h"

namespace Block
{
	struct BlockInfo
	{
		void* dev;

		u8 (*Name)(void* dev, u8* buffer);
		u32 (*Size)(void* dev);

		u32 (*BlockSize)(void* dev);
		u8 (*Read)(void* dev, u32 lba, u8* buffer);
		u8 (*Write)(void* dev, u32 lba, u8* buffer);
	};

	extern LinkedList<BlockInfo*> devices;

	bool Init();

	void Register(BlockInfo* info);
	void Unregister(BlockInfo* info);
}
