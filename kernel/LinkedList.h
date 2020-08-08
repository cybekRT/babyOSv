#include"Memory.h"

template<class X>
class LinkedListItem
{
public:
	X value;
	LinkedListItem<X>* next;
};

template<class X>
class LinkedList
{
public:
	LinkedListItem<X>* data;

	void PushBack(X value)
	{
		LinkedListItem<X>* item = (LinkedListItem<X>*)Memory::Malloc(sizeof(*item));

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

	/*void Remove(X value)
	{

	}*/

	/*void Remove(LinkedListItem<X> item)
	{

	}*/

	X PopFront()
	{
		ASSERT(!IsEmpty(), "Pop from empty linked list");

		auto item = data;
		data = item->next;
		auto itemData = item->value;
		Memory::Mfree(item);
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