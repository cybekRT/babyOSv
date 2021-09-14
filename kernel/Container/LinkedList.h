#pragma once

#include"Memory.h"
#include"Iterator.hpp"

#define offsetof(type, member) \
		((unsigned)(&(((type*)0)->member)))

#define container_of(ptr, type, member) \
	({ \
		const typeof( ((type*)0)->member) *__mptr = (ptr); \
		(type *)( (char*)__mptr - offsetof(type, member) ); \
	})

template<class X>
class LinkedListItem
{
public:
	X value;
	LinkedListItem<X>* next;

	LinkedListItem<X>() : /*value(X()),*/ next(nullptr)
	{
		//Print("> %s\n", __FUNCTION__);
	}

	~LinkedListItem<X>()
	{
		//Print("> %s\n", __FUNCTION__);
	}
};

template<class X>
class LinkedList
{
protected:
	template<class T = LinkedListItem<X>>
	class Iterator : public OneWayIterator<T>
	{
		private:
			T* ptr;

		public:
			Iterator(T* ptr) : ptr(ptr)
			{
				Print("LinkedIterator: %p\n", ptr);
			}

			virtual T& operator*() override
			{
				//LinkedListItem<T> *item = container_of(ptr, LinkedListItem<T>, value);
				//return item->value;
				Print("*itr - %p (%x)\n", ptr, *ptr);
				return *ptr;
			}

			virtual const Iterator operator+(int v) const
			{
				//auto p = ptr;
				LinkedListItem<T> *item = container_of(ptr, LinkedListItem<T>, value);
				for(unsigned a = 0; a < v; a++)
					item = item->next;
				if(!item)
					return Iterator<T>(nullptr);
				else
					return Iterator(&item->value);
			}

			virtual Iterator& operator++() override
			{
				//ptr = ptr->next;
				LinkedListItem<T> *item = container_of(ptr, LinkedListItem<T>, value);
				item = item->next;
				if(!item)
					ptr = nullptr;
				else
					ptr = &item->value;
				return *this;
			}

			virtual const Iterator operator++(int)
			{
				Iterator prev = *this;
				LinkedListItem<T> *item = container_of(ptr, LinkedListItem<T>, value);
				item = item->next;
				if(!item)
					ptr = nullptr;
				else
					ptr = &(item->next)->value;
				return prev;
			}

			virtual Iterator& operator+=(int v) override
			{
				LinkedListItem<T> *item = container_of(ptr, LinkedListItem<T>, value);
				for(unsigned a = 0; a < v; a++)
					item = item->next;
				
				if(!item)
					ptr = nullptr;
				else
					ptr = &item->value;
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
	LinkedListItem<X>* data;

	LinkedList<X>() : data(nullptr)
	{
		Print("LinkedList<X>()\n");
	}

	LinkedList<X>(const LinkedList<X>& arg) : data(nullptr)
	{
		Print("LinkedList<X>(&arg)\n");
		auto ptr = arg.data;
		while(ptr)
		{
			PushBack(ptr->value);
			ptr = ptr->next;
		}
	}

	~LinkedList<X>()
	{
		Print("~LinkedList<X>()\n");
		//Clear();
		Print("...~LinkedList<X>()\n");
	}

	Iterator<X> begin()
	{
		return Iterator<X>(data ? &data->value : nullptr);
	}

	Iterator<X> end()
	{
		return Iterator<X>(nullptr);
	}

	void Clear()
	{
		while(data)
		{
			//Remove(data);
		}
	}

	void PushBack(X value)
	{
		//LinkedListItem<X>* item = (LinkedListItem<X>*)Memory::Alloc(sizeof(*item));
		//auto item = new LinkedListItem<X>();// Memory::Alloc<LinkedListItem<X>>();
		auto item = Memory::Alloc<LinkedListItem<X>>();

		item->value = value;
		item->next = nullptr;

		if(data == nullptr)
		{
			data = item;
		}
		else
		{
			LinkedListItem<X>* ptr = data;
			while(ptr->next != nullptr)
			{
				ASSERT(ptr != ptr->next, "LinkedList next pointing to itself");

				ptr = ptr->next;
			}

			ptr->next = item;
		}

		ASSERT((u32)data != (u32)-1, "Data pointer corrupted~!");
	}

	void Remove(LinkedListItem<X>* item)
	{
		if(!item)
			return;

		if(data == item)
		{
			data = item->next;
			//Memory::Free(item);
			delete item;
			return;
		}

		//Print("Data: %p != Item: %p\n", data, item);
		//return;

		LinkedListItem<X>* ptrPrev = nullptr;
		LinkedListItem<X>* ptr = data;
		while(data)
		{
			if(ptr == item)
			{
				ptrPrev->next = item->next;
				//Memory::Free(item);
				delete item;
				return;
			}

			ptrPrev = data;
			data = data->next;
		}

		ASSERT((u32)data != (u32)-1, "Data pointer corrupted~!");
	}

	X PopFront()
	{
		ASSERT(!IsEmpty(), "Pop from empty linked list");

		auto item = data;
		data = item->next;
		auto itemData = item->value;
		//Memory::Free(item);
		delete item;

		ASSERT((u32)data != (u32)-1, "Data pointer corrupted~!");
		return itemData;
	}

	X PopBack()
	{
		ASSERT(!IsEmpty(), "Pop from empty linked list");

		auto item = data;
		typeof(item) prevItem = nullptr;
		while(item->next != nullptr)
		{
			prevItem = item;
			item = item->next;
		}

		prevItem->next = nullptr;
		auto itemData = item->value;
		//Memory::Free(item);
		delete item;

		ASSERT((u32)data != (u32)-1, "Data pointer corrupted~!");
		return itemData;
	}

	X Front()
	{
		ASSERT(!IsEmpty(), "Pop from empty linked list");
		return data->value;
	}

	X Back()
	{
		ASSERT(!IsEmpty(), "Pop from empty linked list");

		auto item = data;
		while(item->next)
			item = item->next;

		return item->value;
	}

	bool IsEmpty()
	{
		return data == nullptr;
	}

	u32 Size()
	{
		ASSERT((u32)data != (u32)-1, "Data pointer corrupted~!");

		u32 count = 0;
		auto ptr = data;
		while(ptr)
		{
			Print("Ptr: %p, Count: %d\n", ptr, count);
			ptr = ptr->next;
			count++;
		}

		return count;
	}
};