#include<gtest/gtest.h>
#include"FS/FAT.hpp"
#include"FS_mock.hpp"

class FSTest : public testing::Test {
public:
	FSTest()
	{
		DummyPartitionOpen();
		DummyPartitionClear();
		FAT::Init();
	}

	~FSTest()
	{
		DummyPartitionClose();
	}
};

TEST_F(FSTest, FAT_Format_Probe)
{
	Status s;

	s = fs->Probe(&dummyPartition);
	EXPECT_NE(s, Status::Success);

	s = fs->Format(&dummyPartition, nullptr);
	EXPECT_EQ(s, Status::Success);

	s = fs->Probe(&dummyPartition);
	EXPECT_EQ(s, Status::Success);

	s = fs->Mount(&dummyPartition, &fsPriv);
	EXPECT_EQ(s, Status::Success);
}