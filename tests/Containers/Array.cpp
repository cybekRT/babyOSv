#include<cstdlib>
#include<cstring>

template<class T>
class Array
{
protected:
	T* data;
	u32 size;
	u32 capacity;

public:
	Array() : data(nullptr), size(0), capacity(0)
	{

	}

	Array(u32 capacity) : size(0), capacity(capacity)
	{
		data = (T*)new u8[sizeof(T) * capacity];
	}

	Array(const Array& arg) : size(arg.size), capacity(arg.capacity)
	{
		data = (T*)new u8[sizeof(T) * capacity];

		for(unsigned a = 0; a < size; a++)
			data[a] = arg.data[a];
	}

	~Array()
	{
		Clear();
		delete[] (u8*)data;
	}

	Array& operator=(const Array& arg)
	{
		if(data)
		{
			Clear();
			if(capacity < arg.size)
			{
				delete[] (u8*)data;
				data = nullptr;

				capacity = arg.capacity;
			}
		}

		if(!data)
			data = (T*)new u8[sizeof(T) * capacity];

		size = arg.size;

		for(unsigned a = 0; a < size; a++)
			data[a] = arg.data[a];

		return *this;
	}

	T& operator[](u32 index)
	{
		if(index >= size)
		{
			// TODO: assert
			T* ptr = nullptr;
			return *ptr;
		}

		return data[index];
	}

	T* Data()
	{
		return data;
	}

	u32 Size()
	{
		return size;
	}

	u32 Capacity()
	{
		return capacity;
	}

	void Insert(u32 index, const T& arg)
	{
		if(index > size + 1)
		{
			// TODO: assert
			return;
		}

		if(size + 1 > capacity)
		{
			if(!Resize())
			{
				// TODO: assert
				return;
			}
		}

		for(unsigned a = size; a > index; a--)
		{
			memcpy((void*)(&data[a]), (void*)(&data[a - 1]), sizeof(T));
			// data[a] = data[a - 1];
		}

		data[index] = arg;
		size++;
	}

	// void Insert(Iterator itr, const T& arg);

	void Remove(u32 index)
	{
		if(index > size)
		{
			// TODO: assert
			return;
		}

		data[index].~T();
		size--;

		for(unsigned a = index; a < size; a++)
		{
			memcpy((void*)(&data[a]), (void*)(&data[a + 1]), sizeof(T));
			// data[a] = data[a + 1];
		}
	}

	// void Remove(Iterator itr);

	void Remove(const T& arg)
	{
		for(unsigned a = 0; a < size; )
		{
			if(data[a] == arg)
				Remove(a);
			else
				a++;
		}
	}

	void Clear()
	{
		for(unsigned a = 0; a < size; a++)
			data[a].~T();
	}

	void PushFront(const T& arg)
	{
		Insert(0, arg);
	}

	T PopFront()
	{
		T obj = data[0];
		Remove(0);
		return obj;
	}

	T& Front()
	{
		if(!size)
		{
			// TODO: assert
			T* ptr = nullptr;
			return *ptr;
		}

		return data[0];
	}

	void PushBack(const T& arg)
	{
		Insert(size, arg);
	}

	T PopBack()
	{
		T obj = data[size - 1];
		Remove(size - 1);
		return obj;
	}

	T& Back()
	{
		if(!size)
		{
			// TODO: assert
			T* ptr = nullptr;
			return *ptr;
		}

		return data[size - 1];
	}

protected:
	bool Resize()
	{
		u32 newCapacity = (capacity > 0) ? capacity * 2 : 8;
		T* newData = (T*)new u8[sizeof(T) * newCapacity];
		if(!newData)
			return false;

		memcpy(newData, data, capacity * sizeof(T));
		delete[] (u8*)data;

		data = newData;
		capacity = newCapacity;

		return true;
	}
};

#include<gtest/gtest.h>

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
	EXPECT_EQ(test.Size(), 0);
	EXPECT_EQ(test.Capacity(), 0);

	EXPECT_EQ(TestSubject::constructorCalls, 0);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0);
	EXPECT_EQ(TestSubject::destructorCalls, 0);
	EXPECT_EQ(TestSubject::assignCalls, 0);
}

TEST_F(ArrayTest, ConstructorCapacity)
{
	Array<TestSubject> test(17);

	EXPECT_NE(test.Data(), nullptr);
	EXPECT_EQ(test.Size(), 0);
	EXPECT_EQ(test.Capacity(), 17);

	EXPECT_EQ(TestSubject::constructorCalls, 0);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0);
	EXPECT_EQ(TestSubject::destructorCalls, 0);
	EXPECT_EQ(TestSubject::assignCalls, 0);
}

TEST_F(ArrayTest, ConstructorCopy)
{
	Array<TestSubject> test1;
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	test1.PushBack(TestSubject());
	Array<TestSubject> test2(test1);

	EXPECT_NE(test1.Data(), nullptr);
	EXPECT_EQ(test1.Size(), 3);

	EXPECT_NE(test1.Data(), test2.Data());
	EXPECT_NE(test2.Data(), nullptr);
	EXPECT_EQ(test1.Size(), test2.Size());

	EXPECT_EQ(TestSubject::constructorCalls, 3);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0);
	EXPECT_EQ(TestSubject::assignCalls, 6);
}

TEST_F(ArrayTest, Destructor)
{
	{
		Array<TestSubject> test;
		test.PushBack(TestSubject());
		test.PushBack(TestSubject());
		test.PushBack(TestSubject());
	}

	EXPECT_EQ(TestSubject::destructorCalls, 6);
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
	EXPECT_EQ(test1.Size(), 3);

	EXPECT_NE(test1.Data(), test2.Data());
	EXPECT_NE(test2.Data(), nullptr);
	EXPECT_EQ(test1.Size(), test2.Size());

	EXPECT_EQ(TestSubject::constructorCalls, 3);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0);
	EXPECT_EQ(TestSubject::assignCalls, 6);
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

TEST_F(ArrayTest, InsertIndexBegin)
{
	Array<int> test;

	test.Insert(0, 1);
	test.Insert(0, 5);
	test.Insert(0, 9);

	EXPECT_EQ(test.Size(), 3);
	EXPECT_EQ(test[2], 1);
	EXPECT_EQ(test[1], 5);
	EXPECT_EQ(test[0], 9);
}

TEST_F(ArrayTest, InsertIndexMiddle)
{
	Array<int> test;

	test.Insert(0, 0);
	test.Insert(0, 0);
	test.Insert(0, 0);
	test.Insert(0, 0);
	test.Insert(0, 0);

	test.Insert(2, 5);

	EXPECT_EQ(test.Size(), 6);
	EXPECT_EQ(test[1], 0);
	EXPECT_EQ(test[2], 5);
	EXPECT_EQ(test[3], 0);
}

TEST_F(ArrayTest, InsertIndexEnd)
{
	Array<int> test(8);

	test.Insert(0, 0);
	test.Insert(0, 0);
	test.Insert(0, 0);
	test.Insert(0, 0);
	test.Insert(0, 0);

	test.Insert(5, 5);

	EXPECT_EQ(test.Size(), 6);
	EXPECT_EQ(test[4], 0);
	EXPECT_EQ(test[5], 5);
}

TEST_F(ArrayTest, InsertIndexOutside)
{
	Array<int> test;

	test.Insert(10, 1);
	test.Insert(10, 5);
	test.Insert(10, 9);

	EXPECT_EQ(test.Size(), 0);
}

// void Insert(u32 index, const T& arg)
// // void Insert(Iterator itr, const T& arg);
// void Remove(u32 index)
// // void Remove(Iterator itr);
// void Remove(const T& arg)
// void Clear()
// void PushFront(const T& arg)
// T PopFront()
// T& Front()
// void PushBack(const T& arg)
// T PopBack()
// T& Back()