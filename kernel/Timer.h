namespace Timer
{
	typedef u64 Time;

	bool Init();

	u64 GetTicks();
	void Delay(Time ms);
}