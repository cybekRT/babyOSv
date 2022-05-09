#include<gtest/gtest.h>
#include"FS/FAT.hpp"
#include"FS_mock.hpp"

// char* rootdirs_names[] = { "dir_1", "dir_2", "dir_3" };
// char* dir2_subdirs_names[] = { "dir_2_1", "dir_2_2", "dir_2_3" };
char* files_names[] = { "file_1", "file_2", "file_3" };

class FS_ops : public testing::Test {
public:
	static void SetUpTestSuite()
	{
		Status s;

		DummyPartitionOpen();
		DummyPartitionClear();
		FAT::Init();

		s = fs->Format(&dummyPartition, nullptr);
		EXPECT_EQ(s, Status::Success);

		s = fs->Probe(&dummyPartition);
		EXPECT_EQ(s, Status::Success);

		s = fs->Mount(&dummyPartition, &fsPriv);
		EXPECT_EQ(s, Status::Success);
	}

	static void TearDownTestSuite()
	{
		DummyPartitionClose();
	}
};

Status ReadDirUntil(FS::Directory* dir, char* name)
{
	Status s;

	FS::DirEntry entry;
	fs->DirectoryRewind(fsPriv, dir);

	while((s = fs->DirectoryRead(fsPriv, dir, &entry)) == Status::Success)
	{
		if(strcmp((char*)entry.name, name) == 0)
			break;
	}

	return s;
}

int CountEntries(FS::Directory* dir)
{
	Status s;
	int count = 0;

	FS::DirEntry entry;
	s = fs->DirectoryRewind(fsPriv, dir);
	EXPECT_EQ(s, Status::Success);

	while(fs->DirectoryRead(fsPriv, dir, &entry) == Status::Success)
	{
		count++;
	}

	return count;
}

// TEST_F(FS_ops, FAT_CreateDirectories)
// {
// 	Status s;

// 	FS::Directory* dir;
// 	s = fs->DirectoryOpenRoot(fsPriv, &dir);
// 	EXPECT_EQ(s, Status::Success);
// 	EXPECT_NE(dir, nullptr);

// 	// Create root dirs
// 	for(auto dir_name : rootdirs_names)
// 	{
// 		s = fs->DirectoryCreate(fsPriv, dir, dir_name);
// 		EXPECT_EQ(s, Status::Success);
// 	}

// 	// s = fs->DirectoryClose(fsPriv, &dir);
// 	// s = fs->DirectoryOpenRoot(fsPriv, &dir);

// 	// EXPECT_EQ(CountEntries(dir), sizeof(dirs_names) / sizeof(*dirs_names));

// 	// Count root dirs
// 	FS::DirEntry entry;
// 	int count = 0;
// 	fs->DirectoryRewind(fsPriv, dir);
// 	while(fs->DirectoryRead(fsPriv, dir, &entry) == Status::Success)
// 	{
// 		if(count > sizeof(rootdirs_names) / sizeof(*rootdirs_names))
// 		{
// 			count = -1;
// 			break;
// 		}

// 		EXPECT_EQ(strcmp((char*)entry.name, rootdirs_names[count]), 0);

// 		count++;
// 	}

// 	EXPECT_EQ(count, sizeof(rootdirs_names) / sizeof(*rootdirs_names));

// 	// Create Dir_2 subdirs
// 	s = ReadDirUntil(dir, rootdirs_names[1]);
// 	EXPECT_EQ(s, Status::Success);

// 	fs->DirectoryFollow(fsPriv, dir);
// 	for(auto subdir_name : dir2_subdirs_names)
// 	{
// 		s = fs->DirectoryCreate(fsPriv, dir, subdir_name);
// 		EXPECT_EQ(s, Status::Success);
// 	}

// 	// Close dir :P
// 	s = fs->DirectoryClose(fsPriv, &dir);
// 	EXPECT_EQ(s, Status::Success);
// }

// TEST_F(FS_ops, FAT_DirectoriesTraverse)
// {
// 	Status s;

// 	FS::Directory* dir;
// 	s = fs->DirectoryOpenRoot(fsPriv, &dir);
// 	EXPECT_EQ(s, Status::Success);
// 	EXPECT_NE(dir, nullptr);

// 	// Go to A
// 	s = ReadDirUntil(dir, rootdirs_names[0]);
// 	EXPECT_EQ(s, Status::Success);
// 	s = fs->DirectoryFollow(fsPriv, dir);
// 	EXPECT_EQ(s, Status::Success);
// 	EXPECT_LE(CountEntries(dir), 2); // ".", ".."

// 	// Go up
// 	s = ReadDirUntil(dir, "..");
// 	EXPECT_EQ(s, Status::Success);
// 	s = fs->DirectoryFollow(fsPriv, dir);
// 	EXPECT_EQ(s, Status::Success);

// 	// Go to B
// 	s = ReadDirUntil(dir, rootdirs_names[0]);
// 	EXPECT_EQ(s, Status::Success);
// 	s = fs->DirectoryFollow(fsPriv, dir);
// 	EXPECT_EQ(s, Status::Success);
// 	EXPECT_LE(CountEntries(dir), 2 + (sizeof(dir2_subdirs_names) / sizeof(*dir2_subdirs_names)));

// 	s = fs->DirectoryClose(fsPriv, &dir);
// 	EXPECT_EQ(s, Status::Success);
// }

// TEST_F(FS_ops, FAT_CreateFiles)
// {
// 	Status s;

// 	FS::Directory* dir;
// 	s = fs->DirectoryOpenRoot(fsPriv, &dir);
// 	EXPECT_EQ(s, Status::Success);
// 	EXPECT_NE(dir, nullptr);

// 	for(auto file_name : files_names)
// 	{
// 		s = fs->FileCreate(fsPriv, dir, file_name);
// 		EXPECT_EQ(s, Status::Success);
// 	}

// 	FS::DirEntry entry;
// 	int count = 0;
// 	s = fs->DirectoryRewind(fsPriv, dir);
// 	EXPECT_EQ(s, Status::Success);

// 	while(fs->DirectoryRead(fsPriv, dir, &entry) == Status::Success)
// 	{
// 		if(entry.isDirectory)
// 			continue;

// 		EXPECT_TRUE(strcmp((char*)entry.name, files_names[count]) == 0);

// 		count++;
// 	}

// 	EXPECT_EQ(count, sizeof(files_names) / sizeof(*files_names));
// }

// TEST_F(FS_ops, FAT_WriteFile)
// {
// 	Status s;

// 	FS::Directory* dir;
// 	s = fs->DirectoryOpenRoot(fsPriv, &dir);

// 	FS::File* file;
// 	ReadDirUntil(dir, files_names[0]);
// 	s = fs->FileOpen(fsPriv, dir,  &file);
// 	EXPECT_EQ(s, Status::Success);

// 	const unsigned bufferSize = 8192;
// 	char* buffer = new char[bufferSize];
// 	for(unsigned a = 0; a < bufferSize; a++)
// 	{
// 		buffer[a] = a / 128;
// 	}

// 	u32 writtenCount = 0;;
// 	s = fs->FileWrite(fsPriv, file, (u8*)buffer, bufferSize, &writtenCount);
// 	EXPECT_EQ(s, Status::Success);
// 	EXPECT_EQ(writtenCount, bufferSize);

// 	s = fs->FileClose(fsPriv, &file);
// 	EXPECT_EQ(s, Status::Success);

// 	fs->DirectoryClose(fsPriv, &dir);
// }

// TEST_F(FS_ops, FAT_ReadFile)
// {
// 	Status s;

// 	FS::Directory* dir;
// 	s = fs->DirectoryOpenRoot(fsPriv, &dir);

// 	FS::File* file;
// 	ReadDirUntil(dir, files_names[0]);
// 	s = fs->FileOpen(fsPriv, dir,  &file);
// 	EXPECT_EQ(s, Status::Success);

// 	// s = fs->FileSetPointer(fsPriv, file, -1);
// 	// EXPECT_EQ(s, Status::Success);

// 	u32 bufferSize = 8192;
// 	u32 readCount;
// 	// s = fs->FileGetPointer(fsPriv, file, &bufferSize);
// 	// EXPECT_EQ(s, Status::Success);

// 	// s = fs->FileSetPointer(fsPriv, file, 0);
// 	// EXPECT_EQ(s, Status::Success);

// 	char* buffer = new char[bufferSize * 2];
// 	s = fs->FileRead(fsPriv, file, (u8*)buffer, bufferSize * 2, &readCount);
// 	EXPECT_EQ(s, Status::Success);
// 	EXPECT_EQ(readCount, bufferSize);

// 	printf("===== read =====\n");
// 	for(unsigned a = 0; a < readCount; a++)
// 	{
// 		printf("%02x", buffer[a]);
// 	}
// 	printf("\n===== /read =====\n");

// 	bool bufferIsOk = true;
// 	for(unsigned a = 0; a < bufferSize; a++)
// 	{
// 		if(buffer[a] != a / 128)
// 		{
// 			bufferIsOk = false;
// 			break;
// 		}
// 	}

// 	EXPECT_TRUE(bufferIsOk);

// 	s = fs->FileClose(fsPriv, &file);
// 	EXPECT_EQ(s, Status::Success);

// 	fs->DirectoryClose(fsPriv, &dir);
// }
