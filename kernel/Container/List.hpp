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
	Item* tail;
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

	List() : head(nullptr), tail(nullptr), size(0)
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

	// T& operator[](u32 index)
	// {
	// 	Item* ptr = head;
	// 	for(unsigned a = 0; a < index; a++)
	// 		ptr = ptr->next;

	// 	return ptr->data;
	// }

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
		if(itr == head)
		{
			Item* item = new Item(arg);
			item->next = head;
			if(head)
				head->prev = item;
			head = item;

			if(tail == item->prev)
				tail = item;

			size++;

			return Iterator(head);
		}
		else
		{
			Item* ptr = head;
			while(itr != ptr->next && ptr->next)
				ptr = ptr->next;

			Item* item = new Item(arg);
			item->prev = ptr;
			item->next = ptr->next;
			ptr->next = item;

			if(tail == item->prev)
				tail = item;

			size++;

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

	Iterator RemoveAt(Iterator itr)
	{
		if(itr == head)
		{
			printf("Removing head~!\n");
			Item* item = head;
			if(tail == head)
				tail = nullptr;
			head = head->next;

			delete item;
			size--;

			return Iterator(head);
		}
		else if(itr == tail)
		{
			auto prev = tail->prev;
			prev->next = nullptr;

			delete tail;
			tail = prev;
			size--;

			return Iterator(nullptr);
		}
		else
		{
			printf("Removing non-head~!\n");
			Item* ptr = head;
			while(itr != ptr)
				ptr = ptr->next;

			auto next = ptr->next;
			ptr->prev->next = next;
			if(next)
				next->prev = ptr->prev;

			delete ptr;
			size--;

			return Iterator(next);
		}
	}

	void Remove(const T& arg)
	{
		auto itr = Iterator(head);
		while(itr != nullptr)
		{
			if(*itr == arg)
			{
				itr = RemoveAt(itr);
			}
			else
				itr++;
		}
	}

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
		tail = nullptr;
	}

	void PushFront(const T& arg)
	{
		Item* item = new Item(arg);
		item->next = head;
		item->prev = nullptr;
		head = item;
		if(tail == nullptr)
			tail = item;

		size++;
	}

	T PopFront()
	{
		T obj = head->data;
		Item* item = head;
		head = head->next;
		if(tail == item)
			tail = nullptr;
		delete item;
		size--;
		return obj;
	}

	T& Front()
	{
		if(!size)
		{
			// TODO: assert
			T* ptr = nullptr;
			// cppcheck-suppress nullPointer
			return *ptr;
		}

		return head->data;
	}

	void PushBack(const T& arg)
	{
		Item* item = new Item(arg);

		if(head == nullptr)
		{
			head = item;
			tail = item;
		}
		else
		{
			tail->next = item;
			item->prev = tail;
			tail = item;
		}

		size++;
	}

	T PopBack()
	{
		Item* ptr = tail;

		if(ptr->prev != nullptr)
			ptr->prev->next = nullptr;
		else if(head == ptr)
			head = nullptr;
		size--;

		tail = tail->prev;

		T obj = ptr->data;
		delete ptr;
		return obj;
	}

	T& Back()
	{
		if(!size)
		{
			// TODO: assert
			T* ptr = nullptr;
			// cppcheck-suppress nullPointer
			return *ptr;
		}

		return tail->data;
	}
};
