#include"Dummy.hpp"
#include"Block.hpp"

namespace Block::Dummy
{
	u8 Name(void* dev, u8* buffer)
	{
		strcpy("dummy", (char*)buffer);

		return 0;
	}

	u32 Size(void* dev)
	{
		return 0;
	}

	u8 Lock(void* dev)
	{
		return 0;
	}

	u8 Unlock(void* dev)
	{
		return 0;
	}

	u32 BlockSize(void* dev)
	{
		return 0;
	}

	u8 Read(void* dev, u32 lba, u8* buffer)
	{
		return 0;
	}

	u8 Write(void* dev, u32 lba, u8* buffer)
	{
		return 0;
	}

	Block::BlockDeviceDriver drv 
	{
		.Name = Name,
		.Size = Size,
		.BlockSize = BlockSize,

		.Lock = Lock,
		.Unlock = Unlock,

		.Read = Read,
		.Write = Write
	};

	bool Init()
	{
		Block::RegisterDevice(DeviceType::Unknown, &drv, nullptr);

		return true;
	}
}
