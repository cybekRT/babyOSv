#pragma once

template<class T, u32 limit>
class RingBuffer
{
protected:
	T objs[limit];
	u32 size;
	u32 read, write;
	u32 dropped;

public:
	RingBuffer() : size(0), read(0), write(0), dropped(0)
	{

	}

	u32 Size()
	{
		return size;
	}

	bool IsEmpty()
	{
		return size == 0;
	}

	bool IsFull()
	{
		return (size == limit);
	}

	void PushBack(const T& arg)
	{
		if(read == write && size > 0)
		{
			dropped++;
			read = (read + 1) % limit;
		}
		else
			size++;

		objs[write] = arg;
		write = (write + 1) % limit;
	}

	T PopFront()
	{
		ASSERT(size > 0, "index out of range");

		size--;

		T v = objs[read];
		read = (read + 1) % limit;
		return v;
	}

	u32 GetDropped()
	{
		return dropped;
	}
};
