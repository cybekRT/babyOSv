#include<cstdlib>
#include<cstring>
#include<gtest/gtest.h>

#include"Containers/Array.hpp"
#include"TestSubject.hpp"

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

TEST_F(ArrayTest, OperatorAssignmentOnArrayWithElements)
{
	Array<TestSubject> test1(4);
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	Array<TestSubject> test2(32);
	test2.PushBack(TestSubject());
	test2.PushBack(TestSubject());
	test2.PushBack(TestSubject());

	TestSubject::constructorCalls = 0;
	TestSubject::destructorCalls = 0;
	TestSubject::constructorCopyOrAssignment = 0;

	auto oldData = test2.Data();
	test2 = test1;
	EXPECT_EQ(TestSubject::constructorCalls, 0u);
	EXPECT_EQ(TestSubject::constructorCopyOrAssignment, 3u);
	EXPECT_EQ(TestSubject::destructorCalls, 3u);

	EXPECT_NE(test1.Data(), test2.Data());
	EXPECT_NE(test2.Data(), nullptr);
	EXPECT_EQ(test2.Data(), oldData);
	EXPECT_EQ(test1.Size(), test2.Size());

	EXPECT_NE(test1.Capacity(), test2.Capacity());
}

TEST_F(ArrayTest, OperatorAssignmentOnArrayWithElementsLowCapacity)
{
	Array<TestSubject> test1(4);
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	Array<TestSubject> test2(4);
	test2.PushBack(TestSubject());
	test2.PushBack(TestSubject());
	test2.PushBack(TestSubject());

	TestSubject::constructorCalls = 0;
	TestSubject::destructorCalls = 0;
	TestSubject::constructorCopyOrAssignment = 0;

	auto oldData = test2.Data();
	test2 = test1;
	EXPECT_EQ(TestSubject::constructorCalls, 0u);
	EXPECT_EQ(TestSubject::constructorCopyOrAssignment, 6u);
	EXPECT_EQ(TestSubject::destructorCalls, 3u);

	EXPECT_NE(test1.Data(), test2.Data());
	EXPECT_NE(test2.Data(), nullptr);
	// EXPECT_NE(test2.Data(), oldData); // Malloc may return the same address :F
	EXPECT_EQ(test1.Size(), test2.Size());

	EXPECT_EQ(test1.Capacity(), test2.Capacity());
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

TEST_F(ArrayTest, OperatorArrayOutsideRange)
{
	Array<int> test;

	test.PushBack(1);

	EXPECT_EQ(test.Size(), 1u);
	EXPECT_EQ(&test[5], nullptr);
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

	EXPECT_EQ(test.Size(), 3u);
}

TEST_F(ArrayTest, InsertIteratorBegin)
{
	Array<int> test;

	test.InsertAt(test.begin(), 1);
	test.InsertAt(test.begin(), 5);
	test.InsertAt(test.begin(), 9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(test[2], 1);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[0], 9);
}

TEST_F(ArrayTest, InsertIteratorMiddle)
{
	Array<int> test;

	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);

	test.InsertAt(test.begin() + 2, 5);

	ASSERT_EQ(test.Size(), 6u);
	EXPECT_EQ(test[1], 0);
	EXPECT_EQ(test[2], 5);
	EXPECT_EQ(test[3], 0);
}

TEST_F(ArrayTest, InsertIteratorEnd)
{
	Array<int> test(8);

	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);

	test.InsertAt(test.end(), 5);

	ASSERT_EQ(test.Size(), 6u);
	EXPECT_EQ(test[4], 0);
	EXPECT_EQ(test[5], 5);
}

TEST_F(ArrayTest, InsertIteratorOutside)
{
	Array<int> test;

	test.InsertAt(test.begin() + 10, 1);
	test.InsertAt(test.begin() + 10, 5);
	test.InsertAt(test.begin() + 10, 9);

	ASSERT_EQ(test.Size(), 3u);
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[2], 9);
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

	ASSERT_EQ(test.Size(), 3u);
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[2], 9);
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

	ASSERT_EQ(test.Size(), 4u);
	EXPECT_EQ(test[0], 1);
	EXPECT_EQ(test[1], 5);
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

	ASSERT_EQ(test.Size(), 4u);
	EXPECT_EQ(test[1], 1);
	EXPECT_EQ(test[2], 9);
}

TEST_F(ArrayTest, RemoveIteratorBeginEnd)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(9);

	test.RemoveAt(test.begin() + 4);

	ASSERT_EQ(test.Size(), 4u);
	EXPECT_EQ(test[2], 5);
	EXPECT_EQ(test[3], 9);
}

TEST_F(ArrayTest, RemoveIteratorEnd)
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

TEST_F(ArrayTest, RemoveIteratorOutside)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(9);

	test.RemoveAt(test.end() + 10);

	ASSERT_EQ(test.Size(), 5u);
	EXPECT_EQ(test[2], 5);
	EXPECT_EQ(test[3], 9);
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

TEST_F(ArrayTest, FrontEmpty)
{
	Array<int> test;

	EXPECT_EQ(test.Size(), 0u);
	EXPECT_EQ(&test.Front(), nullptr);
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

TEST_F(ArrayTest, BackEmpty)
{
	Array<int> test;

	EXPECT_EQ(test.Size(), 0u);
	EXPECT_EQ(&test.Back(), nullptr);
}

/********** Iterators **********/

TEST_F(ArrayTest, IteratorOperatorDot)
{
	Array<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	EXPECT_EQ(*test.begin(), test[0]);
}

TEST_F(ArrayTest, IteratorOperatorArrow)
{
	Array<TestSubject*> test;
	test.PushBack(new TestSubject());
	test.PushBack(new TestSubject());

	EXPECT_EQ(test.begin()->destructorCalls, 0);

	delete test.Back();
	test.PopBack();
	EXPECT_EQ(test.begin()->destructorCalls, 1);

	delete test.Back();
	test.PopBack();

	EXPECT_EQ(TestSubject::destructorCalls, 2);
}

TEST_F(ArrayTest, IteratorOperatorAddSubtract)
{
	Array<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr = test.begin();
	itr += 2;
	EXPECT_EQ(itr, test.end());

	itr -= 2;
	EXPECT_EQ(itr, test.begin());

	itr -= -2;
	EXPECT_EQ(itr, test.end());
	itr += -2;
	EXPECT_EQ(itr, test.begin());
}

TEST_F(ArrayTest, IteratorOperatorEquals)
{
	Array<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr = test.begin();
	EXPECT_TRUE(itr == test.begin());

	itr = test.end();
	EXPECT_TRUE(itr == test.end());
}

TEST_F(ArrayTest, IteratorOperatorPreIncrement)
{
	Array<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr1 = test.begin();
	auto itr2 = ++itr1;
	EXPECT_EQ(itr1, itr2);
}

TEST_F(ArrayTest, IteratorOperatorPostIncrement)
{
	Array<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr1 = test.begin();
	auto itr2 = itr1++;
	EXPECT_NE(itr1, itr2);
	EXPECT_EQ(itr1, itr2 + 1);
}

TEST_F(ArrayTest, IteratorOperatorPreDecrement)
{
	Array<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr1 = test.end();
	auto itr2 = --itr1;
	EXPECT_EQ(itr1, itr2);
}

TEST_F(ArrayTest, IteratorOperatorPostDecrement)
{
	Array<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr1 = test.end();
	auto itr2 = itr1--;
	EXPECT_NE(itr1, itr2);
	EXPECT_EQ(itr1, itr2 - 1);
}
