#pragma once

namespace Timer
{
	typedef u64 Time;

	bool Init();

	Time GetTicks();
	void Delay(Time ms);

	template<typename T>
	bool WaitUntil(T func, u32 timeoutMs)
	{
		auto ticks = Timer::GetTicks();

		while(Timer::GetTicks() < ticks + timeoutMs)
		{
			if(func())
				return true;
		}

		return false;
	}
}

#define WAIT_UNTIL(arg, timeout) Timer::WaitUntil([&]() -> bool { return (arg); }, (timeout))
