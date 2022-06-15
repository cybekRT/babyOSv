#pragma once

#include"Array.hpp"

template<class K, class V>
class Dictionary
{
protected:
	struct Tuple
	{
		K key;
		V value;
	};

	Array<Tuple> keys;

public:
	Dictionary()
	{

	}

	~Dictionary()
	{
		Clear();
	}

	V& operator[](const K& key)
	{
		if(Tuple* t = FindTuple(key))
		{
			return t->value;
		}

		return Insert(key, V())->value;
	}

	auto begin()
	{
		return keys.begin();
	}

	auto end()
	{
		return keys.end();
	}

	u32 Size()
	{
		return keys.Size();
	}

	bool IsEmpty()
	{
		return keys.Size() == 0;
	}

	void Clear()
	{
		keys.Clear();
	}

	void RemoveKey(const K& key)
	{
		auto ptr = FindTuple(key);
		if(ptr)
		{
			u32 index = (u32)(ptr - &*keys.begin());
			keys.RemoveAt(index);
		}
	}

	void Print()
	{
		printf("=== Dictionary ===\n");
		for(auto v : keys)
		{
			printf("- %d\n", v.key);
		}
		printf("=== /Dictionary ===\n");
	}

protected:
	Tuple* FindTuple(const K& key)
	{
		if(keys.IsEmpty())
			return nullptr;

		u32 is = 0, ie = keys.Size();
		u32 index;
		while(is != ie && is < ie)
		{
			index = (is + ie) / 2;
			if(key < keys[index].key)
				ie = index - 1;
			else if(keys[index].key < key)
				is = index + 1;
			else
				return &keys[index];
		}

		if(is >= 0 && is < keys.Size() && key == keys[is].key)
			return &keys[is];

		return nullptr;
	}

	Tuple* Insert(const K& key, const V& value)
	{
		if(keys.IsEmpty())
		{
			keys.PushBack(Tuple { .key = key, .value = value } );
			return &keys.Back();
		}

		u32 is = 0, ie = keys.Size();
		u32 index;
		while(is != ie && is < ie)
		{
			index = (is + ie) / 2;
			if(key < keys[index].key)
				ie = index - 1;
			else if(key == keys[index].key)
			{
				keys[index].value = value;
				return &keys[index];
			}
			else
				is = index + 1;
		}

		if(is >= keys.Size() || key < keys[is].key)
			index = is;
		else
			index = is+1;

		keys.InsertAt(index, Tuple { .key = key, .value = value } );
		return &keys[index];
	}
};