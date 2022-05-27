#include<cstdlib>
#include<cstring>
#include<gtest/gtest.h>

#include"Containers/RingBuffer.hpp"

TEST(RingBufferTest, Constructor)
{
	RingBuffer<int, 8> test;

	EXPECT_EQ(test.Size(), 0u);
}

TEST(RingBufferTest, Fill)
{
	RingBuffer<int, 8> test;

	EXPECT_EQ(test.Size(), 0u);

	for(unsigned a = 0; a < 8; a++)
		test.PushBack(a);

	EXPECT_EQ(test.Size(), 8u);
	EXPECT_EQ(test.GetDropped(), 0u);
}

TEST(RingBufferTest, Drop)
{
	RingBuffer<int, 8> test;

	EXPECT_EQ(test.Size(), 0u);

	for(unsigned a = 0; a < 10; a++)
		test.PushBack(a);

	EXPECT_EQ(test.Size(), 8u);
	EXPECT_EQ(test.GetDropped(), 2u);
}

TEST(RingBufferTest, PushPop)
{
	RingBuffer<int, 8> test;

	int read = 0, write = 0;

	for(unsigned a = 0; a < 8; a++)
		test.PushBack(a);

	for(unsigned a = 0; a < 4; a++)
		EXPECT_EQ(test.PopFront(), a);

	for(unsigned a = 8; a < 12; a++)
		test.PushBack(a);

	for(unsigned a = 4; a < 8; a++)
		EXPECT_EQ(test.PopFront(), a);

	EXPECT_EQ(test.Size(), 4u);
	EXPECT_EQ(test.GetDropped(), 0u);
}
