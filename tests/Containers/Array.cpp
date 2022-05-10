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
	class Iterator
	{
	protected:
		T* ptr;

	public:
		Iterator(T* ptr) : ptr(ptr)
		{

		}

		Iterator(const Iterator& arg) : ptr(arg.ptr)
		{

		}

		T& operator*()
		{
			return *ptr;
		}

		T& operator->()
		{
			return *ptr;
		}

		bool operator==(const Iterator& arg) const
		{
			return ptr == arg.ptr;
		}

		bool operator!=(const Iterator& arg) const
		{
			return ptr != arg.ptr;
		}

		Iterator operator+(int arg) const
		{
			return  Iterator(ptr + arg);
		}

		Iterator& operator++()
		{
			Next();
		}

		Iterator operator++(int)
		{
			auto itr = *this;
			Next();
			return itr;
		}

		const Iterator& operator+=(int arg)
		{
			if(arg >= 0)
			{
				for(int a = 0; a < arg; a++)
					Next();
			}
			else
			{
				for(int a = 0; a > arg; a--)
					Prev();
			}

			return *this;
		}

		Iterator operator-(int arg) const
		{
			return Iterator(ptr - arg);
		}

		Iterator& operator--()
		{
			Prev();
		}

		Iterator operator--(int)
		{
			auto itr = *this;
			Prev();
			return itr;
		}

		const Iterator& operator-=(int arg)
		{
			if(arg >= 0)
			{
				for(int a = 0; a < arg; a++)
					Prev();
			}
			else
			{
				for(int a = 0; a > arg; a--)
					Next();
			}

			return *this;
		}

	protected:
		void Next()
		{
			ptr++;
		}

		void Prev()
		{
			ptr--;
		}
	};

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

	Iterator begin()
	{
		return Iterator(data);
	}

	Iterator end()
	{
		return Iterator(data + size);
	}

	void InsertAt(u32 index, const T& arg)
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

	Iterator InsertAt(Iterator itr, const T& arg)
	{
		T* ptr = &(*itr);
		auto index = ptr - data;
		InsertAt(index, arg);

		return Iterator(data + index);
	}

	void RemoveAt(u32 index)
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

	Iterator RemoveAt(Iterator itr)
	{
		T* ptr = &(*itr);
		auto index = ptr - data;
		RemoveAt(index);

		return Iterator(data + index);
	}

	void Remove(const T& arg)
	{
		for(unsigned a = 0; a < size; )
		{
			if(data[a] == arg)
				RemoveAt(a);
			else
				a++;
		}
	}

	void Clear()
	{
		for(unsigned a = 0; a < size; a++)
			data[a].~T();

		size = 0;
	}

	void PushFront(const T& arg)
	{
		InsertAt(0, arg);
	}

	T PopFront()
	{
		T obj = data[0];
		RemoveAt(0);
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
		InsertAt(size, arg);
	}

	T PopBack()
	{
		T obj = data[size - 1];
		RemoveAt(size - 1);
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

	int a = 0;
	for(auto itr = test.begin(); itr != test.end(); )
	{
		itr = test.RemoveAt(itr);

		a++;
		EXPECT_EQ(test.Size(), 4 - a);
	}

	EXPECT_EQ(a, 4);
	EXPECT_EQ(test.Size(), 0);
}

TEST_F(ArrayTest, ForeachIterator_Iterate)
{
	Array<int> test;
	int vals[] = { 5, 1, 3, 9 };

	for(auto val : vals)
	{
		test.PushBack(val);
	}

	int a = 0;
	for(auto itr : test)
	{
		EXPECT_EQ(itr, vals[a]);

		a++;
	}

	EXPECT_EQ(a, 4);
}

TEST_F(ArrayTest, InsertIndexBegin)
{
	Array<int> test;

	test.InsertAt(0, 1);
	test.InsertAt(0, 5);
	test.InsertAt(0, 9);

	EXPECT_EQ(test.Size(), 3);
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

	EXPECT_EQ(test.Size(), 6);
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

	EXPECT_EQ(test.Size(), 6);
	EXPECT_EQ(test[4], 0);
	EXPECT_EQ(test[5], 5);
}

TEST_F(ArrayTest, InsertIndexOutside)
{
	Array<int> test;

	test.InsertAt(10, 1);
	test.InsertAt(10, 5);
	test.InsertAt(10, 9);

	EXPECT_EQ(test.Size(), 0);
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

	EXPECT_EQ(test.Size(), 4);
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

	EXPECT_EQ(test.Size(), 4);
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

	EXPECT_EQ(test.Size(), 4);
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

	EXPECT_EQ(test.Size(), 4);
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

	EXPECT_EQ(test.Size(), 2);
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

	EXPECT_EQ(test.Size(), 2);
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

	EXPECT_EQ(test.Size(), 2);
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

	EXPECT_EQ(test.Size(), 3);
}

// // void Remove(Iterator itr);

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

	EXPECT_EQ(test.Size(), 6);
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

	EXPECT_EQ(test.Size(), 9);
}

TEST_F(ArrayTest, Clear)
{
	Array<TestSubject> test;
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());
	test.PushBack(TestSubject());

	EXPECT_NE(test.Data(), nullptr);
	EXPECT_EQ(test.Size(), 3);

	EXPECT_EQ(TestSubject::constructorCalls, 3);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0);
	EXPECT_EQ(TestSubject::assignCalls, 3);
	EXPECT_EQ(TestSubject::destructorCalls, 3);

	test.Clear();

	EXPECT_NE(test.Data(), nullptr);
	EXPECT_EQ(test.Size(), 0);

	EXPECT_EQ(TestSubject::constructorCalls, 3);
	EXPECT_EQ(TestSubject::constructorCopyCalls, 0);
	EXPECT_EQ(TestSubject::assignCalls, 3);
	EXPECT_EQ(TestSubject::destructorCalls, 6);
}

TEST_F(ArrayTest, PushFront)
{
	Array<int> test;

	test.PushFront(1);
	test.PushFront(5);
	test.PushFront(9);

	EXPECT_EQ(test.Size(), 3);
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

	EXPECT_EQ(test.Size(), 3);
	EXPECT_EQ(test.PopFront(), 9);
	EXPECT_EQ(test.PopFront(), 5);
	EXPECT_EQ(test.PopFront(), 1);
	EXPECT_EQ(test.Size(), 0);
}

TEST_F(ArrayTest, Front)
{
	Array<int> test;

	test.PushFront(1);
	EXPECT_EQ(test.Size(), 1);
	EXPECT_EQ(test.Front(), 1);

	test.PushFront(5);
	EXPECT_EQ(test.Size(), 2);
	EXPECT_EQ(test.Front(), 5);

	test.PushFront(9);
	EXPECT_EQ(test.Size(), 3);
	EXPECT_EQ(test.Front(), 9);

	EXPECT_EQ(test.Size(), 3);
}

TEST_F(ArrayTest, PushBack)
{
	Array<int> test;

	test.PushBack(1);
	test.PushBack(5);
	test.PushBack(9);

	EXPECT_EQ(test.Size(), 3);
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

	EXPECT_EQ(test.Size(), 3);
	EXPECT_EQ(test.PopBack(), 9);
	EXPECT_EQ(test.PopBack(), 5);
	EXPECT_EQ(test.PopBack(), 1);
	EXPECT_EQ(test.Size(), 0);
}

TEST_F(ArrayTest, Back)
{
	Array<int> test;

	test.PushBack(1);
	EXPECT_EQ(test.Size(), 1);
	EXPECT_EQ(test.Back(), 1);

	test.PushBack(5);
	EXPECT_EQ(test.Size(), 2);
	EXPECT_EQ(test.Back(), 5);

	test.PushBack(9);
	EXPECT_EQ(test.Size(), 3);
	EXPECT_EQ(test.Back(), 9);

	EXPECT_EQ(test.Size(), 3);
}
