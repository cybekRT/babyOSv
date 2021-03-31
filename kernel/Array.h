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
		objs = (T*)Memory::Malloc(capacity * sizeof(T));
	}

	/*~Array()
	{
		Memory::Mfree(objs);
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

	void PushBack(const T& v)
	{
		if(size + 1 > capacity)
			Realloc();

		objs[size] = v;
		size++;
	}

	void PopBack()
	{
		ASSERT(size > 0, "Pop from empty array");

		size--;
	}

	void RemoteAt(u32 index)
	{
		ASSERT(index < size, "RemoveAt invalid index");

		memcpy(objs + index, objs + index + 1, (size - index) * sizeof(T));
		size--;
	}

private:
	void Realloc()
	{
		if(capacity < 8)
			capacity = 8;
		else
			capacity *= 2;

		T* newObjs = (T*)Memory::Malloc(capacity * sizeof(T));
		ASSERT(newObjs != nullptr, "Can't alloc array");

		if(objs)
		{
			memcpy(newObjs, objs, size * sizeof(T));
			Memory::Mfree(objs);
		}

		objs = newObjs;
	}
};
