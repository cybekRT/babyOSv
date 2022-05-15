namespace Timer
{
	typedef u64 Time;

	bool Init();

	Time GetTicks();
	void Delay(Time ms);
}