#pragma once

//#include"Memory.hpp"
#include"Iterator.hpp"
#include"IContainer.hpp"

#ifndef offsetof
#define offsetof(type, member) \
		((unsigned)(&(((type*)0)->member)))
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
	({ \
		const typeof( ((type*)0)->member) *__mptr = (ptr); \
		(type *)( (char*)__mptr - offsetof(type, member) ); \
	})
#endif

namespace Container
{
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
	class LinkedList : public IContainer<X>
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
			
		}

		LinkedList<X>(const LinkedList<X>& arg) : data(nullptr)
		{
			auto ptr = arg.data;
			while(ptr)
			{
				PushBack(ptr->value);
				ptr = ptr->next;
			}
		}

		~LinkedList<X>()
		{
			Clear();
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
			__asm("pushf\ncli");
			while(data)
			{
				Remove(data);
			}

			__asm("popf");
		}

		void PushBack(const X& value)
		{
			__asm("pushf\ncli");
			auto item = new LinkedListItem<X>();

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
			__asm("popf");
		}

		void PushFront(const X& value)
		{
			__asm("pushf\ncli");
			auto item = new LinkedListItem<X>();

			item->value = value;
			item->next = data;
			data = item;

			ASSERT((u32)data != (u32)-1, "Data pointer corrupted~!");
			__asm("popf");
		}

		void Remove(LinkedListItem<X>* item)
		{
			if(!item)
			{
				Print("Not removed~!\n");
				return;
			}
			__asm("pushf\ncli");

			if(data == item)
			{
				//Print("Removing first item~!\n");
				data = item->next;
				delete item;
				// Print("Removed 1?\n");
				__asm("popf");
				return;
			}

			LinkedListItem<X>* ptrPrev = nullptr;
			LinkedListItem<X>* ptr = data;
			while(ptr)
			{
				if(ptr == item)
				{
					// Print("Removed 2? %p, %p, %p\n", ptrPrev, data, item->next);
					ptrPrev->next = item->next;
					delete item;
					__asm("popf");
					return;
				}

				ptrPrev = ptr;
				ptr = ptr->next;
			}

			// Print("Removed 3?\n");
			ASSERT((u32)data != (u32)-1, "Data pointer corrupted~!");
			__asm("popf");
		}

		X PopFront()
		{
			__asm("pushf\ncli");
			ASSERT(!IsEmpty(), "Pop from empty linked list");

			auto item = data;
			data = item->next;
			auto itemData = item->value;
			delete item;

			ASSERT((u32)data != (u32)-1, "Data pointer corrupted~!");
			__asm("popf");
			return itemData;
		}

		X PopBack()
		{
			__asm("pushf\ncli");
			ASSERT(!IsEmpty(), "Pop from empty linked list");

			auto item = data;
			typeof(item) prevItem = nullptr;
			while(item->next != nullptr)
			{
				prevItem = item;
				item = item->next;
			}

			if(prevItem)
				prevItem->next = nullptr;
			else
				data = nullptr;
			auto itemData = item->value;
			delete item;

			ASSERT((u32)data != (u32)-1, "Data pointer corrupted~!");
			__asm("popf");
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
				ptr = ptr->next;
				count++;
			}

			return count;
		}
	};
}
