#include<cstdlib>
#include<cstring>
#include<gtest/gtest.h>

#include"Container/List.hpp"
#include"TestSubject.hpp"

class ListTest : public testing::Test {
public:
	ListTest()
	{
		TestSubject::constructorCalls = 0;
		TestSubject::constructorCopyCalls = 0;
		TestSubject::destructorCalls = 0;
		TestSubject::assignCalls = 0;
	}

	~ListTest()
	{

	}
};

TEST_F(ListTest, Constructor)
{
	List<TestSubject> test;

	EXPECT_EQ(test.Size(), 0u);

	EXPECT_EQ(TestSubject::constructorCalls, 0u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0u);
	EXPECT_EQ(TestSubject::destructorCalls, 0u);
	EXPECT_EQ(TestSubject::assignCalls, 0u);
}

TEST_F(ListTest, ConstructorCopy)
{
	List<TestSubject> test1;
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	List<TestSubject> test2(test1);

	EXPECT_EQ(test1.Size(), 3u);
	EXPECT_EQ(test1.Size(), test2.Size());

	EXPECT_EQ(TestSubject::constructorCalls, 3u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 6u);
	EXPECT_EQ(TestSubject::assignCalls, 0u);
}

TEST_F(ListTest, Destructor)
{
	{
		List<TestSubject> test;
		test.PushBack(TestSubject());
		test.PushBack(TestSubject());
		test.PushBack(TestSubject());
	}

	EXPECT_EQ(TestSubject::destructorCalls, 6u);
}

TEST_F(ListTest, OperatorAssignment)
{
	List<TestSubject> test1;
	List<TestSubject> test2;
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());

	test2 = test1;

	EXPECT_EQ(test1.Size(), 3u);
	EXPECT_EQ(test1.Size(), test2.Size());

	EXPECT_EQ(TestSubject::constructorCalls, 3u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 6u);
	EXPECT_EQ(TestSubject::assignCalls, 0u);
}

TEST_F(ListTest, ForeachIterator_Iterate)
{
	List<int> test;
	int vals[] = { 5, 1, 3, 9 };

	for(auto val : vals)
	{
		test.PushBack(val);
	}

	unsigned a = 0;
	for(auto itr : test)
	{
		EXPECT_EQ(itr, vals[a]);

		a++;
	}

	EXPECT_EQ(a, 4u);
}

TEST_F(ListTest, Clear)
{
	List<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	EXPECT_EQ(test.Size(), 3u);

	EXPECT_EQ(TestSubject::constructorCalls, 3u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 3u);
	EXPECT_EQ(TestSubject::assignCalls, 0u);
	EXPECT_EQ(TestSubject::destructorCalls, 3u);

	test.Clear();

	EXPECT_EQ(test.Size(), 0u);

	EXPECT_EQ(TestSubject::constructorCalls, 3u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 3u);
	EXPECT_EQ(TestSubject::assignCalls, 0u);
	EXPECT_EQ(TestSubject::destructorCalls, 6u);
}

TEST_F(ListTest, PushFront)
{
	List<int> test;

	test.PushFront(1);
	test.PushFront(5);
	test.PushFront(9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test[0], 9);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[2], 1);
}

TEST_F(ListTest, PopFront)
{
	List<int> test;

	test.PushFront(1);
	test.PushFront(5);
	test.PushFront(9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test.PopFront(), 9);
	EXPECT_EQ(test.PopFront(), 5);
	EXPECT_EQ(test.PopFront(), 1);
	EXPECT_EQ(test.Size(), 0u);
}

TEST_F(ListTest, Front)
{
	List<int> test;

	test.PushFront(1);
	EXPECT_EQ(test.Size(), 1u);
	EXPECT_EQ(test.Front(), 1);

	test.PushFront(5);
	EXPECT_EQ(test.Size(), 2u);
	EXPECT_EQ(test.Front(), 5);

	test.PushFront(9);
	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test.Front(), 9);

	EXPECT_EQ(test.Size(), 3u);
}

TEST_F(ListTest, PushBack)
{
	List<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[2], 9);
}

TEST_F(ListTest, PopBack)
{
	List<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test.PopBack(), 9);
	EXPECT_EQ(test.PopBack(), 5);
	EXPECT_EQ(test.PopBack(), 1);
	EXPECT_EQ(test.Size(), 0u);
}

TEST_F(ListTest, Back)
{
	List<int> test;

	test.PushBack(1);
	EXPECT_EQ(test.Size(), 1u);
	EXPECT_EQ(test.Back(), 1);

	test.PushBack(5);
	EXPECT_EQ(test.Size(), 2u);
	EXPECT_EQ(test.Back(), 5);

	test.PushBack(9);
	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test.Back(), 9);

	EXPECT_EQ(test.Size(), 3u);
}
