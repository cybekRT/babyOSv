#include<cstdlib>
#include<cstring>
#include<gtest/gtest.h>

#include"Container/Array.hpp"

class TestSubject
{
public:
	static unsigned constructorCalls;
	static unsigned constructorCopyCalls;
	static unsigned destructorCalls;
	static unsigned assignCalls;

	TestSubject()
	{
		constructorCalls++;
	}

	TestSubject(const TestSubject& arg)
	{
		constructorCopyCalls++;
	}

	~TestSubject()
	{
		destructorCalls++;
	}

	TestSubject& operator=(const TestSubject& arg)
	{
		assignCalls++;
		return *this;
	}
};

unsigned TestSubject::constructorCalls = 0;
unsigned TestSubject::constructorCopyCalls = 0;
unsigned TestSubject::destructorCalls = 0;
unsigned TestSubject::assignCalls = 0;

class ArrayTest : public testing::Test {
public:
	ArrayTest()
	{
		TestSubject::constructorCalls = 0;
		TestSubject::constructorCopyCalls = 0;
		TestSubject::destructorCalls = 0;
		TestSubject::assignCalls = 0;
	}

	~ArrayTest()
	{

	}
};

TEST_F(ArrayTest, Constructor)
{
	Array<TestSubject> test;

	EXPECT_EQ(test.Data(), nullptr);
	EXPECT_EQ(test.Size(), 0u);
	EXPECT_EQ(test.Capacity(), 0u);

	EXPECT_EQ(TestSubject::constructorCalls, 0u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0u);
	EXPECT_EQ(TestSubject::destructorCalls, 0u);
	EXPECT_EQ(TestSubject::assignCalls, 0u);
}

TEST_F(ArrayTest, ConstructorCapacity)
{
	Array<TestSubject> test(17);
	Array<TestSubject> test2(12345);

	EXPECT_NE(test.Data(), nullptr);
	EXPECT_EQ(test.Size(), 0u);
	EXPECT_EQ(test.Capacity(), 17u);

	// This will NOT change the capacity, is it really a good design? :|
	// test = test2;
	// EXPECT_NE(test.Data(), nullptr);
	// EXPECT_EQ(test.Size(), 0u);
	// EXPECT_EQ(test.Capacity(), 12345u);

	EXPECT_NE(test2.Data(), nullptr);
	EXPECT_EQ(test2.Size(), 0u);
	EXPECT_EQ(test2.Capacity(), 12345u);

	EXPECT_EQ(TestSubject::constructorCalls, 0u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0u);
	EXPECT_EQ(TestSubject::destructorCalls, 0u);
	EXPECT_EQ(TestSubject::assignCalls, 0u);
}

TEST_F(ArrayTest, ConstructorCopy)
{
	Array<TestSubject> test1;
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	Array<TestSubject> test2(test1);

	EXPECT_NE(test1.Data(), nullptr);
	EXPECT_EQ(test1.Size(), 3u);

	EXPECT_NE(test1.Data(), test2.Data());
	EXPECT_NE(test2.Data(), nullptr);
	EXPECT_EQ(test1.Size(), test2.Size());

	EXPECT_EQ(TestSubject::constructorCalls, 3u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0u);
	EXPECT_EQ(TestSubject::assignCalls, 6u);
}

TEST_F(ArrayTest, Destructor)
{
	{
		Array<TestSubject> test;
		test.PushBack(TestSubject());
		test.PushBack(TestSubject());
		test.PushBack(TestSubject());
	}

	EXPECT_EQ(TestSubject::destructorCalls, 6u);
}

TEST_F(ArrayTest, OperatorAssignment)
{
	Array<TestSubject> test1;
	Array<TestSubject> test2;
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());

	test2 = test1;

	EXPECT_NE(test1.Data(), nullptr);
	EXPECT_EQ(test1.Size(), 3u);

	EXPECT_NE(test1.Data(), test2.Data());
	EXPECT_NE(test2.Data(), nullptr);
	EXPECT_EQ(test1.Size(), test2.Size());

	EXPECT_EQ(TestSubject::constructorCalls, 3u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0u);
	EXPECT_EQ(TestSubject::assignCalls, 6u);
}

TEST_F(ArrayTest, OperatorArray)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(2);

	EXPECT_EQ(test.Size(), 4u);
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[2], 9);
	EXPECT_EQ(test[3], 2);
}

TEST_F(ArrayTest, ForeachIterator_Empty)
{
	Array<int> test;

	EXPECT_EQ(test.begin(), Array<int>::Iterator(nullptr));
	EXPECT_EQ(test.end(), Array<int>::Iterator(nullptr));
}

TEST_F(ArrayTest, ForeachIterator_BeginEndEqual)
{
	Array<int> test;
	test.PushBack(5);

	EXPECT_NE(test.begin(), Array<int>::Iterator(nullptr));
	EXPECT_NE(test.end(), Array<int>::Iterator(nullptr));
	EXPECT_EQ(test.begin() + 1, test.end());
	EXPECT_EQ(test.begin(), test.end() - 1);
}

TEST_F(ArrayTest, ForeachIterator_Remove)
{
	Array<int> test;
	test.PushBack(5);
	test.PushBack(1);
	test.PushBack(3);
	test.PushBack(9);

	unsigned a = 0;
	for(auto itr = test.begin(); itr != test.end(); )
	{
		itr = test.RemoveAt(itr);

		a++;
		EXPECT_EQ(test.Size(), 4u - a);
	}

	EXPECT_EQ(a, 4u);
	EXPECT_EQ(test.Size(), 0u);
}

TEST_F(ArrayTest, ForeachIterator_Iterate)
{
	Array<int> test;
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

TEST_F(ArrayTest, InsertIndexBegin)
{
	Array<int> test;

	test.InsertAt(0, 1);
	test.InsertAt(0, 5);
	test.InsertAt(0, 9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test[2], 1);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[0], 9);
}

TEST_F(ArrayTest, InsertIndexMiddle)
{
	Array<int> test;

	test.InsertAt(0, 0);
	test.InsertAt(0, 0);
	test.InsertAt(0, 0);
	test.InsertAt(0, 0);
	test.InsertAt(0, 0);

	test.InsertAt(2, 5);

	EXPECT_EQ(test.Size(), 6u);
	EXPECT_EQ(test[1], 0);
	EXPECT_EQ(test[2], 5);
	EXPECT_EQ(test[3], 0);
}

TEST_F(ArrayTest, InsertIndexEnd)
{
	Array<int> test(8);

	test.InsertAt(0, 0);
	test.InsertAt(0, 0);
	test.InsertAt(0, 0);
	test.InsertAt(0, 0);
	test.InsertAt(0, 0);

	test.InsertAt(5, 5);

	EXPECT_EQ(test.Size(), 6u);
	EXPECT_EQ(test[4], 0);
	EXPECT_EQ(test[5], 5);
}

TEST_F(ArrayTest, InsertIndexOutside)
{
	Array<int> test;

	test.InsertAt(10, 1);
	test.InsertAt(10, 5);
	test.InsertAt(10, 9);

	EXPECT_EQ(test.Size(), 0u);
}

TEST_F(ArrayTest, RemoveIteratorBegin)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(9);

	test.RemoveAt(test.begin());
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 5);

	EXPECT_EQ(test.Size(), 4u);
}

TEST_F(ArrayTest, RemoveIteratorMiddle)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(9);

	test.RemoveAt(test.begin() + 2);
	EXPECT_EQ(test[1], 1);
	EXPECT_EQ(test[2], 9);

	EXPECT_EQ(test.Size(), 4u);
}

TEST_F(ArrayTest, RemoveIteratorEnd)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(9);

	test.RemoveAt(test.begin() + 4);
	EXPECT_EQ(test[2], 5);
	EXPECT_EQ(test[3], 9);

	EXPECT_EQ(test.Size(), 4u);
}

TEST_F(ArrayTest, RemoveIteratorEnd2)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(9);

	test.RemoveAt(test.end() - 1);
	EXPECT_EQ(test[2], 5);
	EXPECT_EQ(test[3], 9);

	EXPECT_EQ(test.Size(), 4u);
}

TEST_F(ArrayTest, RemoveIndexBegin)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	test.RemoveAt(0);
	EXPECT_EQ(test[0], 5);
	EXPECT_EQ(test[1], 9);

	EXPECT_EQ(test.Size(), 2u);
}

TEST_F(ArrayTest, RemoveIndexMiddle)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	test.RemoveAt(1);
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 9);

	EXPECT_EQ(test.Size(), 2u);
}

TEST_F(ArrayTest, RemoveIndexEnd)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	test.RemoveAt(2);
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 5);

	EXPECT_EQ(test.Size(), 2u);
}

TEST_F(ArrayTest, RemoveIndexOutside)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	test.RemoveAt(12345);
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[2], 9);

	EXPECT_EQ(test.Size(), 3u);
}

TEST_F(ArrayTest, RemoveObject_Hit)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	test.Remove(1);
	EXPECT_NE(test[0], 1);
	EXPECT_NE(test[3], 1);

	EXPECT_EQ(test.Size(), 6u);
}

TEST_F(ArrayTest, RemoveObject_Miss)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	test.Remove(7);
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[2], 9);
	EXPECT_EQ(test[3], 1);

	EXPECT_EQ(test.Size(), 9u);
}

TEST_F(ArrayTest, Clear)
{
	Array<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	EXPECT_NE(test.Data(), nullptr);
	EXPECT_EQ(test.Size(), 3u);

	EXPECT_EQ(TestSubject::constructorCalls, 3u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0u);
	EXPECT_EQ(TestSubject::assignCalls, 3u);
	EXPECT_EQ(TestSubject::destructorCalls, 3u);

	test.Clear();

	EXPECT_NE(test.Data(), nullptr);
	EXPECT_EQ(test.Size(), 0u);

	EXPECT_EQ(TestSubject::constructorCalls, 3u);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0u);
	EXPECT_EQ(TestSubject::assignCalls, 3u);
	EXPECT_EQ(TestSubject::destructorCalls, 6u);
}

TEST_F(ArrayTest, PushFront)
{
	Array<int> test;

	test.PushFront(1);
	test.PushFront(5);
	test.PushFront(9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test[0], 9);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[2], 1);
}

TEST_F(ArrayTest, PopFront)
{
	Array<int> test;

	test.PushFront(1);
	test.PushFront(5);
	test.PushFront(9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test.PopFront(), 9);
	EXPECT_EQ(test.PopFront(), 5);
	EXPECT_EQ(test.PopFront(), 1);
	EXPECT_EQ(test.Size(), 0u);
}

TEST_F(ArrayTest, Front)
{
	Array<int> test;

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

TEST_F(ArrayTest, PushBack)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[2], 9);
}

TEST_F(ArrayTest, PopBack)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test.PopBack(), 9);
	EXPECT_EQ(test.PopBack(), 5);
	EXPECT_EQ(test.PopBack(), 1);
	EXPECT_EQ(test.Size(), 0u);
}

TEST_F(ArrayTest, Back)
{
	Array<int> test;

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
