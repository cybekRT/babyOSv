#pragma once

class Mutex
{
	private:
		u32 count;

	public:
		Mutex();
		~Mutex();

		void Lock();
		void Unlock();
		bool IsReady();
};
