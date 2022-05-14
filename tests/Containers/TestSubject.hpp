#pragma once

class TestSubject
{
public:
	static unsigned constructorCalls;
	static unsigned constructorCopyCalls;
	static unsigned destructorCalls;
	static unsigned assignCalls;
	static unsigned constructorCopyOrAssignment;

	TestSubject()
	{
		constructorCalls++;
	}

	TestSubject(const TestSubject& arg)
	{
		constructorCopyCalls++;
		constructorCopyOrAssignment++;
	}

	~TestSubject()
	{
		destructorCalls++;
	}

	TestSubject& operator=(const TestSubject& arg)
	{
		assignCalls++;
		constructorCopyOrAssignment++;
		return *this;
	}

	bool operator==(const TestSubject& arg) const
	{
		return this == &arg;
	}
};
