#pragma once

template<class T, class X>
class IteratorBase
{
protected:
	T* ptr;

public:
	IteratorBase(T* ptr) : ptr(ptr)
	{

	}

	IteratorBase(const IteratorBase& arg) : ptr(arg.ptr)
	{

	}

	T& operator*() const
	{
		return *ptr;
	}

	T& operator->() const
	{
		return *ptr;
	}

	bool operator==(const X& arg) const
	{
		return ptr == arg.ptr;
	}

	bool operator!=(const X& arg) const
	{
		return ptr != arg.ptr;
	}

	X operator+(int arg) const
	{
		auto tmp = X(ptr);
		tmp += arg;
		return tmp;
	}

	X& operator++()
	{
		Next();
		return *(X*)this;
	}

	X operator++(int)
	{
		X itr = *(X*)this;
		Next();
		return itr;
	}

	const X& operator+=(int arg)
	{
		if(arg >= 0)
		{
			for(int a = 0; a < arg; a++)
				Next();
		}
		else
		{
			for(int a = 0; a > arg; a--)
				Prev();
		}

		return *(X*)this;
	}

	X operator-(int arg) const
	{
		auto tmp = X(ptr);
		tmp -= arg;
		return tmp;
	}

	X& operator--()
	{
		Prev();
		return *(X*)this;
	}

	X operator--(int)
	{
		X itr = *(X*)this;
		Prev();
		return itr;
	}

	const X& operator-=(int arg)
	{
		if(arg >= 0)
		{
			for(int a = 0; a < arg; a++)
				Prev();
		}
		else
		{
			for(int a = 0; a > arg; a--)
				Next();
		}

		return *(X*)this;
	}

protected:
	virtual void Next() = 0;

	virtual void Prev() = 0;
};
