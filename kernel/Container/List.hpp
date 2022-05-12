#pragma once

#include"IteratorBase.hpp"

template<class T>
class List
{
protected:
	class Item
	{
	public:
		T data;
		Item* prev;
		Item* next;

		Item(const T& arg) : data(arg), prev(nullptr), next(nullptr)
		{

		}
	};

	Item* head;
	unsigned size;

public:
	class Iterator : public IteratorBase<Item, Iterator>
	{
	public:
		Iterator(Item* ptr) : IteratorBase<Item, Iterator>(ptr)
		{

		}

		Iterator(const Iterator& arg) : IteratorBase<Item, Iterator>(arg)
		{

		}

		Iterator& operator=(const Iterator& arg)
		{
			this->ptr = arg.ptr;
			return *this;
		}

		Item& operator*()
		{
			return this->ptr->data;
		}

		Item& operator->()
		{
			return this->ptr->data;
		}
	protected:
		void Next() override
		{
			this->ptr = this->ptr->next;
		}

		void Prev() override
		{
			this->ptr = this->ptr->prev;
		}
	};

	List() : head(nullptr), size(0)
	{

	}

	List(const List& arg) : List()
	{
		for(const T& item : arg)
			this->PushBack(item);
	}

	~List()
	{
		Clear();
	}

	List& operator=(const Array& arg)
	{
		Clear();

		for(const T& item : arg)
			this->PushBack(item);

		return *this;
	}

	u32 Size()
	{
		return size;
	}

	Iterator begin()
	{
		return Iterator(head);
	}

	Iterator end()
	{
		return Iterator(nullptr);
	}

	// void InsertAt(u32 index, const T& arg)
	// {
	// 	if(index > size + 1)
	// 	{
	// 		// TODO: assert
	// 		return;
	// 	}

	// 	if(size + 1 > capacity)
	// 	{
	// 		if(!Resize())
	// 		{
	// 			// TODO: assert
	// 			return;
	// 		}
	// 	}

	// 	for(unsigned a = size; a > index; a--)
	// 	{
	// 		memcpy((void*)(&data[a]), (void*)(&data[a - 1]), sizeof(T));
	// 		// data[a] = data[a - 1];
	// 	}

	// 	data[index] = arg;
	// 	size++;
	// }

	// Iterator InsertAt(Iterator itr, const T& arg)
	// {
	// 	T* ptr = &(*itr);
	// 	auto index = ptr - data;
	// 	InsertAt(index, arg);

	// 	return Iterator(data + index);
	// }

	// void RemoveAt(u32 index)
	// {
	// 	if(index > size)
	// 	{
	// 		// TODO: assert
	// 		return;
	// 	}

	// 	data[index].~T();
	// 	size--;

	// 	for(unsigned a = index; a < size; a++)
	// 	{
	// 		memcpy((void*)(&data[a]), (void*)(&data[a + 1]), sizeof(T));
	// 		// data[a] = data[a + 1];
	// 	}
	// }

	// Iterator RemoveAt(Iterator itr)
	// {
	// 	T* ptr = &(*itr);
	// 	auto index = ptr - data;
	// 	RemoveAt(index);

	// 	return Iterator(data + index);
	// }

	// void Remove(const T& arg)
	// {
	// 	for(unsigned a = 0; a < size; )
	// 	{
	// 		if(data[a] == arg)
	// 			RemoveAt(a);
	// 		else
	// 			a++;
	// 	}
	// }

	void Clear()
	{
		// for(unsigned a = 0; a < size; a++)
		// 	data[a].~T();

		size = 0;
		head = nullptr;
	}

	void PushFront(const T& arg)
	{
		Item* item = new Item(arg);
		// InsertAt(0, arg);
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
};
