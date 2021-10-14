#pragma once

#include"Memory.h"

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
public:
	LinkedListItem<X>* data;

	LinkedList<X>() : data(nullptr)
	{

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
	}

	X PopFront()
	{
		ASSERT(!IsEmpty(), "Pop from empty linked list");

		auto item = data;
		data = item->next;
		auto itemData = item->value;
		Memory::Free(item);
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
		u32 count = 0;
		auto ptr = data;
		while(ptr)
		{
			ptr = ptr->next;
			count++;
		}

		return count;
	}
};