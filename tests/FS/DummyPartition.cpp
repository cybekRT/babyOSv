// #include"DummyPartition.hpp"
// #include<fstream>
// #include<cstring>

// const u32 blockSize = 512;
// const u32 lbaCount = 2000;

// u8 Name(void* dev, u8* buffer)
// {
// 	return 0;
// }

// u32 Size(void* dev)
// {
// 	return lbaCount;
// }

// u32 BlockSize(void* dev)
// {
// 	return blockSize;
// }

// u8 Lock(void* dev)
// {
// 	return 0;
// }

// u8 Unlock(void* dev)
// {
// 	return 0;
// }

// u8 Read(void* dev, u32 lba, u8* buffer)
// {
// 	return 0;
// }

// u8 Write(void* dev, u32 lba, u8* buffer)
// {
// 	return 0;
// }

// Block::BlockDeviceDriver dummyDeviceDriver
// {
// 		.Name = Name,
// 		.Size = Size,
// 		.BlockSize = BlockSize,
// 		.Lock = Lock,
// 		.Unlock = Unlock,
// 		.Read = Read,
// 		.Write = Write,
// };

// Block::BlockDevice dummyBlockDevice
// {
// 	.type = Block::DeviceType::HardDrive,
// 	.drv = &dummyDeviceDriver,
// 	.drvPriv = nullptr,
// };

// Block::BlockPartition dummyPartition
// {
// 	.type = Block::PartitionType::Raw,
// 	.device = &dummyBlockDevice,
// 	.deviceLbaOffset = 0,
// 	.lbaCount = 2000,
// 	//.name = "DummyPartition"
// };

// const char* fPath = "dummy_part.tst";
// std::fstream f;

// char* fBuffer = nullptr;

// u8 Block::BlockPartition::Read(u32 lba, u8* buffer)
// {
// 	// f.seekg(lba * 512, std::ios::beg);
// 	// f.read((char*)buffer, 512);

// 	memcpy((char*)buffer, fBuffer + lba * 512, 512);

// 	return 0;
// }

// u8 Block::BlockPartition::Write(u32 lba, u8* buffer)
// {
// 	// f.seekp(lba * 512, std::ios::beg);
// 	// f.write((char*)buffer, 512);

// 	memcpy(fBuffer + lba * 512, (char*)buffer, 512);

// 	return 0;
// }

// bool DummyPartitionOpen()
// {
// 	// f.open(fPath, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
// 	// if(!f)
// 	// 	return false;

// 	// char tmp[blockSize] = { 0 };
// 	// for(unsigned a = 0; a < lbaCount; a++)
// 	// 	f.write(tmp, blockSize);

// 	if(!fBuffer)
// 	{
// 		fBuffer = new char[blockSize * lbaCount];
// 		memset(fBuffer, 0, blockSize * lbaCount);
// 	}

// 	return true;
// }

// bool DummyPartitionClose()
// {
// 	std::ofstream f("out_tests/part.img", std::ios::binary);
// 	f.write(fBuffer, blockSize * lbaCount);
// 	f.close();
// 	// bool failed = f.fail();

// 	// f.close();

// 	// return !failed;

// 	// delete[] fBuffer;
// 	return true;
// }

// bool DummyPartitionClear()
// {
// 	memset(fBuffer, 0, blockSize * lbaCount);

// 	return true;
// }
