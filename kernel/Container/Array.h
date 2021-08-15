#pragma once

#include"Memory.h"
#include"Iterator.hpp"

template<class T>
class Array
{
	T* objs;
	u32 capacity;
	u32 size;

	class Iterator : public TwoWayIterator<T>
	{
		private:
			T* ptr;

		public:
			Iterator(T* ptr) : ptr(ptr)
			{
				
			}

			virtual T& operator*() override
			{
				return *ptr;
			}

			virtual const Iterator operator+(int v) const
			{
				return Iterator(ptr + v);
			}

			virtual Iterator& operator++() override
			{
				ptr++;
				return *this;
			}

			virtual const Iterator operator++(int)
			{
				Iterator prev = *this;
				ptr++;
				return prev;
			}

			virtual Iterator& operator+=(int v) override
			{
				ptr += v;
				return *this;
			}

			virtual const Iterator operator-(int v) const
			{
				return Iterator(ptr - v);
			}

			virtual Iterator& operator--() override
			{
				ptr--;
				return *this;
			}

			virtual const Iterator operator--(int)
			{
				Iterator prev = *this;
				ptr--;
				return prev;
			}

			virtual Iterator& operator-=(int v) override
			{
				ptr -= v;
				return *this;
			}

			virtual bool operator==(const OneWayIterator<T> &arg) override
			{
				return (ptr == static_cast<const Iterator*>(&arg)->ptr);
			}

			virtual bool operator!=(const OneWayIterator<T> &arg) override
			{
				return (ptr != static_cast<const Iterator*>(&arg)->ptr);
			}
	};

public:
	Array() : objs(nullptr), capacity(0), size(0)
	{

	}

	Array(u32 capacity) : capacity(capacity), size(0)
	{
		objs = (T*)Memory::Alloc(capacity * sizeof(T));
	}

	~Array()
	{
		Memory::Free(objs);
	}

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

	Iterator begin()
	{
		return Iterator(objs);
	}

	Iterator end()
	{
		return Iterator(objs + size);
	}

	void Insert(Iterator itr, const T& v)
	{
		u32 pos = (u32)((&(*itr)) - (&(*begin())));

		if(size + 1 > capacity)
			Realloc();

		if(size > 0)
		{
			for(unsigned a = size - 1; a > pos; a--)
			{
				objs[a] = objs[a - 1];
				if(a == 0)
					break;
			}
		}

		objs[pos] = v;
		size++;
	}

	void PushFront(const T& v)
	{
		Insert(begin(), v);
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
		Insert(end(), v);
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
		Print("Realloc...\n");
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
