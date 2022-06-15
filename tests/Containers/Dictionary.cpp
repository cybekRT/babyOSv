#include<cstdlib>
#include<cstring>
#include<gtest/gtest.h>

#include"Containers/Dictionary.hpp"

TEST(DictionaryTest, Constructor)
{
	Dictionary<int, int> test;
	EXPECT_EQ(test.Size(), 0u);
}

TEST(DictionaryTest, OperatorArrayInsert)
{
	Dictionary<int, int> test;

	printf("## Insert 0\n");
	test[0] = 0;
	printf("## Insert 123\n");
	test[123] = 123;
	printf("## Insert 5\n");
	test[5] = 5;

	printf("## Check size\n");
	ASSERT_EQ(test.Size(), 3u);

	printf("## Check test[0]\n");
	EXPECT_EQ(test[0], 0);
	printf("## Check test[123]\n");
	EXPECT_EQ(test[123], 123);
	printf("## Check test[5]\n");
	EXPECT_EQ(test[5], 5);

	printf("## Check test[7]\n");
	EXPECT_EQ(test[7], 0);
}

TEST(DictionaryTest, OperatorArrayReplace)
{
	Dictionary<int, int> test;

	test[3] = 0;
	test[123] = 1234;
	test[3] = 5;

	ASSERT_EQ(test.Size(), 2u);

	EXPECT_EQ(test[3], 5);
	EXPECT_EQ(test[123], 1234);
}

TEST(DictionaryTest, Clear)
{
	Dictionary<int, int> test;

	test[3] = 0;
	test[123] = 1234;
	test[6] = 5;

	ASSERT_EQ(test.IsEmpty(), false);
	ASSERT_EQ(test.Size(), 3u);
	test.Clear();
	ASSERT_EQ(test.Size(), 0u);
	ASSERT_EQ(test.IsEmpty(), true);
}

TEST(DictionaryTest, RemoveKey)
{
	Dictionary<int, int> test;

	test[3] = 0;
	test[123] = 1234;
	test[6] = 5;

	test.RemoveKey(3);

	ASSERT_EQ(test.Size(), 2u);
	EXPECT_EQ(test[3], 0);
	EXPECT_EQ(test[123], 1234);
	EXPECT_EQ(test[6], 5);
	ASSERT_EQ(test.Size(), 3u);

	test.RemoveKey(6);
	test.RemoveKey(123);
	ASSERT_EQ(test.Size(), 1u);

	test.RemoveKey(3);
	ASSERT_EQ(test.Size(), 0u);
}
