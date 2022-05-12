#pragma once

class TestSubject
{
public:
	static unsigned constructorCalls;
	static unsigned constructorCopyCalls;
	static unsigned destructorCalls;
	static unsigned assignCalls;

	TestSubject()
	{
		constructorCalls++;
	}

	TestSubject(const TestSubject& arg)
	{
		constructorCopyCalls++;
	}

	~TestSubject()
	{
		destructorCalls++;
	}

	TestSubject& operator=(const TestSubject& arg)
	{
		assignCalls++;
		return *this;
	}
};
