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

		explicit Item(const T& arg) : data(arg), prev(nullptr), next(nullptr)
		{

		}
	};

	Item* head;
	unsigned size;

public:
	class Iterator : public IteratorBase<Item, Iterator>
	{
	public:
		explicit Iterator(Item* ptr) : IteratorBase<Item, Iterator>(ptr)
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

		T& operator*()
		{
			return this->ptr->data;
		}

		T& operator->()
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

	List& operator=(const List& arg)
	{
		Clear();

		for(const T& item : arg)
			this->PushBack(item);

		return *this;
	}

	T& operator[](u32 index)
	{
		Item* ptr = head;
		for(unsigned a = 0; a < index; a++)
			ptr = ptr->next;

		return ptr->data;
	}

	u32 Size()
	{
		return size;
	}

	Iterator begin() const
	{
		return Iterator(head);
	}

	Iterator end() const
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

	Iterator InsertAt(Iterator itr, const T& arg)
	{
		if(&head->data == itr)
		{
			Item* item = new Item(arg);
			item->next = head;
			if(head)
				head->prev = item;
			head = item;

			return Iterator(head);
		}
		else
		{
			Item* ptr = head;
			while(&ptr->next->data != itr)
				ptr = ptr->next;

			Item* item = new Item(arg);
			item->prev = ptr;
			item->next = ptr->next;
			ptr->next = item;

			return Iterator(item);
		}
	}

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
		while(head)
		{
			Item* ptr = head->next;
			delete head;
			head = ptr;
		}

		size = 0;
		// head = nullptr;
	}

	void PushFront(const T& arg)
	{
		Item* item = new Item(arg);
		item->next = head;
		item->prev = nullptr;
		head = item;

		size++;
	}

	T PopFront()
	{
		T obj = head->data;
		Item* item = head;
		head = head->next;
		delete item;
		size--;
		return obj;
	}

	T& Front()
	{
		if(!size)
		{
			// TODO: assert
			// T* ptr = nullptr;
			// return *ptr;
		}

		return head->data;
	}

	void PushBack(const T& arg)
	{
		Item* item = new Item(arg);

		if(head == nullptr)
		{
			head = item;
		}
		else
		{
			Item* ptr = head;
			while(ptr->next != nullptr)
				ptr = ptr->next;
			ptr->next = item;
			item->prev = ptr;
		}

		size++;
	}

	T PopBack()
	{
		Item* ptr = head;
		while(ptr->next != nullptr)
			ptr = ptr->next;

		if(ptr->prev != nullptr)
			ptr->prev->next = nullptr;
		else if(head == ptr)
			head = nullptr;
		size--;

		T obj = ptr->data;
		delete ptr;
		return obj;
	}

	T& Back()
	{
		if(!size)
		{
			// TODO: assert
			// T* ptr = nullptr;
			// return *ptr;
		}

		Item* ptr = head;
		while(ptr->next != nullptr)
			ptr = ptr->next;

		return ptr->data;
	}
};
