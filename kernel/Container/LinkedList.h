#pragma once

#include"Memory.h"
#include"Iterator.hpp"

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
				
			}

			virtual T& operator*() override
			{
				return *ptr;
			}

			virtual const Iterator operator+(int v) const
			{
				auto p = ptr;
				for(unsigned a = 0; a < v; a++)
					p = p->next;
				return Iterator(p);
			}

			virtual Iterator& operator++() override
			{
				ptr = ptr->next;
				return *this;
			}

			virtual const Iterator operator++(int)
			{
				Iterator prev = *this;
				ptr = ptr->next;
				return prev;
			}

			virtual Iterator& operator+=(int v) override
			{
				for(unsigned a = 0; a < v; a++)
					ptr = ptr->next;
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

	}

	Iterator<LinkedListItem<X>> begin()
	{
		return Iterator(data);
	}

	Iterator<LinkedListItem<X>> end()
	{
		return Iterator<LinkedListItem<X>>(nullptr);
	}

	void PushBack(X value)
	{
		//LinkedListItem<X>* item = (LinkedListItem<X>*)Memory::Alloc(sizeof(*item));
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
		if(data == item)
		{
			data = item->next;
			Memory::Free(item);
			return;
		}

		LinkedListItem<X>* ptrPrev = nullptr;
		LinkedListItem<X>* ptr = data;
		while(data)
		{
			if(ptr == item)
			{
				ptrPrev->next = item->next;
				Memory::Free(item);
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
		Memory::Free(item);

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
		Memory::Free(item);

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