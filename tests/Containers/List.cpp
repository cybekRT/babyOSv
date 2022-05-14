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

TEST_F(ListTest, ForeachIterator_Empty)
{
	List<int> test;

	EXPECT_EQ(test.begin(), List<int>::Iterator(nullptr));
	EXPECT_EQ(test.end(), List<int>::Iterator(nullptr));
}

TEST_F(ListTest, ForeachIterator_BeginEndEqual)
{
	List<int> test;
	test.PushBack(5);

	EXPECT_NE(test.begin(), List<int>::Iterator(nullptr));
	EXPECT_EQ(test.end(), List<int>::Iterator(nullptr));
	EXPECT_EQ(test.begin() + 1, test.end());
	// EXPECT_EQ(test.begin(), test.end() - 1);
}

TEST_F(ListTest, ForeachIterator_Remove)
{
	List<int> test;
	test.PushBack(5);
	test.PushBack(1);
	test.PushBack(3);
	test.PushBack(9);

	unsigned a = 0;
	for(auto itr = test.begin(); itr != test.end(); )
	{
		printf("RemoveAt~~~!\n");
		itr = test.RemoveAt(itr);

		a++;
		EXPECT_EQ(test.Size(), 4u - a);
	}

	EXPECT_EQ(a, 4u);
	EXPECT_EQ(test.Size(), 0u);
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

// TEST_F(ListTest, InsertIndexBegin)
// {
// 	List<int> test;

// 	test.InsertAt(0, 1);
// 	test.InsertAt(0, 5);
// 	test.InsertAt(0, 9);

// 	EXPECT_EQ(test.Size(), 3u);
// 	EXPECT_EQ(test[2], 1);
// 	EXPECT_EQ(test[1], 5);
// 	EXPECT_EQ(test[0], 9);
// }

// TEST_F(ListTest, InsertIndexMiddle)
// {
// 	List<int> test;

// 	test.InsertAt(0, 0);
// 	test.InsertAt(0, 0);
// 	test.InsertAt(0, 0);
// 	test.InsertAt(0, 0);
// 	test.InsertAt(0, 0);

// 	test.InsertAt(2, 5);

// 	EXPECT_EQ(test.Size(), 6u);
// 	EXPECT_EQ(test[1], 0);
// 	EXPECT_EQ(test[2], 5);
// 	EXPECT_EQ(test[3], 0);
// }

// TEST_F(ListTest, InsertIndexEnd)
// {
// 	List<int> test(8);

// 	test.InsertAt(0, 0);
// 	test.InsertAt(0, 0);
// 	test.InsertAt(0, 0);
// 	test.InsertAt(0, 0);
// 	test.InsertAt(0, 0);

// 	test.InsertAt(5, 5);

// 	EXPECT_EQ(test.Size(), 6u);
// 	EXPECT_EQ(test[4], 0);
// 	EXPECT_EQ(test[5], 5);
// }

// TEST_F(ListTest, InsertIndexOutside)
// {
// 	List<int> test;

// 	test.InsertAt(10, 1);
// 	test.InsertAt(10, 5);
// 	test.InsertAt(10, 9);

// 	EXPECT_EQ(test.Size(), 0u);
// }

TEST_F(ListTest, InsertIteratorBegin)
{
	List<int> test;

	test.InsertAt(test.begin(), 1);
	test.InsertAt(test.begin(), 5);
	test.InsertAt(test.begin(), 9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(*(test.begin() + 2), 1);
	EXPECT_EQ(*(test.begin() + 1), 5);
	EXPECT_EQ(*(test.begin() + 0), 9);
}

TEST_F(ListTest, InsertIteratorMiddle)
{
	List<int> test;

	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);

	test.InsertAt(test.begin() + 2, 5);

	ASSERT_EQ(test.Size(), 6u);
	EXPECT_EQ(*(test.begin() + 1), 0);
	EXPECT_EQ(*(test.begin() + 2), 5);
	EXPECT_EQ(*(test.begin() + 3), 0);
}

TEST_F(ListTest, InsertIteratorEnd)
{
	List<int> test;

	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);
	test.InsertAt(test.begin(), 0);

	test.InsertAt(test.end(), 5);

	ASSERT_EQ(test.Size(), 6u);
	EXPECT_EQ(*(test.begin() + 4), 0);
	EXPECT_EQ(*(test.begin() + 5), 5);
}

TEST_F(ListTest, RemoveIteratorBegin)
{
	List<int> test;

	test.PushBack(1);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(9);

	test.RemoveAt(test.begin());

	ASSERT_EQ(test.Size(), 4u);
	EXPECT_EQ(*(test.begin() + 0), 1);
	EXPECT_EQ(*(test.begin() + 1), 5);
}

TEST_F(ListTest, RemoveIteratorMiddle)
{
	List<int> test;

	test.PushBack(1);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(9);

	test.RemoveAt(test.begin() + 2);

	ASSERT_EQ(test.Size(), 4u);
	EXPECT_EQ(*(test.begin() + 1), 1);
	EXPECT_EQ(*(test.begin() + 2), 9);
}

TEST_F(ListTest, RemoveIteratorBeginEnd)
{
	List<int> test;

	test.PushBack(1);
	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);
	test.PushBack(9);

	test.RemoveAt(test.begin() + 4);

	ASSERT_EQ(test.Size(), 4u);
	EXPECT_EQ(*(test.begin() + 2), 5);
	EXPECT_EQ(*(test.begin() + 3), 9);
}

// TEST_F(ListTest, RemoveIteratorEnd)
// {
// 	List<int> test;

// 	test.PushBack(1);
// 	test.PushBack(1);
// 	test.PushBack(5);
// 	test.PushBack(9);
// 	test.PushBack(9);

// 	test.RemoveAt(test.end() - 1);
// 	EXPECT_EQ(test[2], 5);
// 	EXPECT_EQ(test[3], 9);

// 	EXPECT_EQ(test.Size(), 4u);
// }

// TEST_F(ListTest, RemoveIndexBegin)
// {
// 	List<int> test;

// 	test.PushBack(1);
// 	test.PushBack(5);
// 	test.PushBack(9);

// 	test.RemoveAt(0);
// 	EXPECT_EQ(test[0], 5);
// 	EXPECT_EQ(test[1], 9);

// 	EXPECT_EQ(test.Size(), 2u);
// }

// TEST_F(ListTest, RemoveIndexMiddle)
// {
// 	List<int> test;

// 	test.PushBack(1);
// 	test.PushBack(5);
// 	test.PushBack(9);

// 	test.RemoveAt(1);
// 	EXPECT_EQ(test[0], 1);
// 	EXPECT_EQ(test[1], 9);

// 	EXPECT_EQ(test.Size(), 2u);
// }

// TEST_F(ListTest, RemoveIndexEnd)
// {
// 	List<int> test;

// 	test.PushBack(1);
// 	test.PushBack(5);
// 	test.PushBack(9);

// 	test.RemoveAt(2);
// 	EXPECT_EQ(test[0], 1);
// 	EXPECT_EQ(test[1], 5);

// 	EXPECT_EQ(test.Size(), 2u);
// }

// TEST_F(ListTest, RemoveIndexOutside)
// {
// 	List<int> test;

// 	test.PushBack(1);
// 	test.PushBack(5);
// 	test.PushBack(9);

// 	test.RemoveAt(12345);
// 	EXPECT_EQ(test[0], 1);
// 	EXPECT_EQ(test[1], 5);
// 	EXPECT_EQ(test[2], 9);

// 	EXPECT_EQ(test.Size(), 3u);
// }

// TEST_F(ListTest, RemoveObject_Hit)
// {
// 	List<int> test;

// 	test.PushBack(1);
// 	test.PushBack(5);
// 	test.PushBack(9);
// 	test.PushBack(1);
// 	test.PushBack(5);
// 	test.PushBack(9);
// 	test.PushBack(1);
// 	test.PushBack(5);
// 	test.PushBack(9);

// 	test.Remove(1);
// 	EXPECT_NE(test[0], 1);
// 	EXPECT_NE(test[3], 1);

// 	EXPECT_EQ(test.Size(), 6u);
// }

TEST_F(ListTest, RemoveObject_Miss)
{
	List<int> test;

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
	EXPECT_EQ(*(test.begin() + 0), 1);
	EXPECT_EQ(*(test.begin() + 1), 5);
	EXPECT_EQ(*(test.begin() + 2), 9);
	EXPECT_EQ(*(test.begin() + 3), 1);

	EXPECT_EQ(test.Size(), 9u);
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
	EXPECT_EQ(*(test.begin() + 0), 9);
	EXPECT_EQ(*(test.begin() + 1), 5);
	EXPECT_EQ(*(test.begin() + 2), 1);
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

TEST_F(ListTest, FrontEmpty)
{
	List<int> test;

	EXPECT_EQ(test.Size(), 0u);
	EXPECT_EQ(&test.Front(), nullptr);
}

TEST_F(ListTest, PushBack)
{
	List<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	EXPECT_EQ(test.Size(), 3u);
	EXPECT_EQ(*(test.begin() + 0), 1);
	EXPECT_EQ(*(test.begin() + 1), 5);
	EXPECT_EQ(*(test.begin() + 2), 9);
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

TEST_F(ListTest, BackEmpty)
{
	List<int> test;

	EXPECT_EQ(test.Size(), 0u);
	EXPECT_EQ(&test.Back(), nullptr);
}

/********** Iterators **********/

TEST_F(ListTest, IteratorOperatorDot)
{
	List<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	EXPECT_EQ(*test.begin(), *(test.begin() + 0));
}

TEST_F(ListTest, IteratorOperatorArrow)
{
	List<TestSubject*> test;
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

TEST_F(ListTest, IteratorOperatorAddSubtract)
{
	List<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr = test.begin();
	itr += 2;
	EXPECT_EQ(itr, test.begin() + 2);

	itr -= 2;
	EXPECT_EQ(itr, test.begin());
}

TEST_F(ListTest, IteratorOperatorEquals)
{
	List<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr = test.begin();
	EXPECT_TRUE(itr == test.begin());

	itr = test.end();
	EXPECT_TRUE(itr == test.end());
}

TEST_F(ListTest, IteratorOperatorPreIncrement)
{
	List<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr1 = test.begin();
	auto itr2 = ++itr1;
	EXPECT_EQ(itr1, itr2);
}

TEST_F(ListTest, IteratorOperatorPostIncrement)
{
	List<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr1 = test.begin();
	auto itr2 = itr1++;
	EXPECT_NE(itr1, itr2);
	EXPECT_EQ(itr1, itr2 + 1);
}

TEST_F(ListTest, IteratorOperatorPreDecrement)
{
	List<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr1 = test.begin() + 2;
	auto itr2 = --itr1;
	EXPECT_EQ(itr1, itr2);
}

TEST_F(ListTest, IteratorOperatorPostDecrement)
{
	List<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	auto itr1 = test.begin() + 2;
	auto itr2 = itr1--;
	EXPECT_NE(itr1, itr2);
	EXPECT_EQ(itr1, itr2 - 1);
}
