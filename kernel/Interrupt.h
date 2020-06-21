namespace Interrupt
{
	bool Init();

	void Register(u8 index, void(*handler)()); // TODO: returning previous handler, so chaining is possible?
	void Unregister(u8 index);
}