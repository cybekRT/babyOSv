#pragma once

#include"IteratorBase.hpp"

template<class T>
class Array
{
protected:
	T* data;
	u32 size;
	u32 capacity;

public:
	class Iterator : public IteratorBase<T, Iterator>
	{
	public:
		explicit Iterator(T* ptr) : IteratorBase<T, Iterator>(ptr)
		{

		}

		Iterator(const Iterator& arg) : IteratorBase<T, Iterator>(arg)
		{

		}

		Iterator& operator=(const Iterator& arg)
		{
			this->ptr = arg.ptr;
			return *this;
		}

	protected:
		void Next() override
		{
			this->ptr++;
		}

		void Prev() override
		{
			this->ptr--;
		}
	};

	Array() : data(nullptr), size(0), capacity(0)
	{

	}

	explicit Array(u32 capacity) : data(nullptr), size(0), capacity(capacity)
	{
		ASSERT(capacity > 0, "Invalid capacity");
		data = (T*)new u8[sizeof(T) * capacity];
	}

	Array(const Array& arg) : data(nullptr), size(arg.size), capacity(arg.capacity)
	{
		if(!arg.data || !arg.size)
			return;

		data = (T*)new u8[sizeof(T) * capacity];

		for(unsigned a = 0; a < size; a++)
			new(data + a) T(arg.data[a]);
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

				capacity = 0;
			}
		}

		if(!arg.data || !arg.size)
			return *this;

		if(!data)
		{
			capacity = arg.capacity;
			data = (T*)new u8[sizeof(T) * capacity];
		}

		size = arg.size;

		for(unsigned a = 0; a < size; a++)
			new(data + a) T(arg.data[a]);

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

	bool IsEmpty()
	{
		return size == 0;
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
		if(index > size)
			index = size;

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

		// Remember: assigning new object to garbage data is not a good idea...
		new(data + index) T(arg);
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

		memcpy((void*)newData, (void*)data, capacity * sizeof(T));
		delete[] (u8*)data;

		data = newData;
		capacity = newCapacity;

		return true;
	}
};
