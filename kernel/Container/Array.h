#pragma once

#include "Memory.h"

template<class T>
class Array
{
	T* objs;
	u32 capacity;
	u32 size;

public:
	Array() : objs(nullptr), capacity(0), size(0)
	{

	}

	Array(u32 capacity) : capacity(capacity), size(0)
	{
		objs = (T*)Memory::Alloc(capacity * sizeof(T));
	}

	/*~Array()
	{
		Memory::Free(objs);
	}*/

	u32 Size()
	{
		return size;
	}

	u32 Capacity()
	{
		return capacity;
	}

	T& operator[](u32 index)
	{
		ASSERT(index < size, "Array[] invalid index");

		return objs[index];
	}

	T* begin()
	{
		return objs;
	}

	T* end()
	{
		return objs + size;
	}

	void PushFront(const T& v)
	{
		if(size + 1 > capacity)
			Realloc();

		size++;
		for(unsigned a = size - 1; a > 0; a--)
			objs[a] = objs[a - 1];;

		objs[0] = v;
	}

	T PopFront()
	{
		ASSERT(size > 0, "Pop from empty array");

		T v = objs[0];
		memcpy(objs + 0, objs + 1, (size - 1) * sizeof(T));
		size--;
		return v;
	}

	void PushBack(const T& v)
	{
		if(size + 1 > capacity)
			Realloc();

		objs[size] = v;
		size++;
	}

	T PopBack()
	{
		ASSERT(size > 0, "Pop from empty array");

		size--;
		return objs[size];
	}

	void RemoteAt(u32 index)
	{
		ASSERT(index < size, "RemoveAt invalid index");

		memcpy(objs + index, objs + index + 1, (size - index - 1) * sizeof(T));
		size--;
	}

private:
	void Realloc()
	{
		if(capacity < 8)
			capacity = 8;
		else
			capacity *= 2;

		T* newObjs = (T*)Memory::Alloc(capacity * sizeof(T));
		ASSERT(newObjs != nullptr, "Can't alloc array");

		if(objs)
		{
			memcpy(newObjs, objs, size * sizeof(T));
			Memory::Free(objs);
		}

		objs = newObjs;
	}
};
