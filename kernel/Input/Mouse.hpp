#include"PS2.hpp"

namespace Mouse
{
	enum class Button
	{
		Left,
		Middle,
		Right
	};

	enum class EventType
	{
		Movement,
		ButtonClick,
		ButtonRelease
	};

	struct Event
	{
		EventType type;
		union
		{
			Button button;
			struct {
				int x;
				int y;
			} movement;
		};
	};

	typedef void (*EventHandler)(const Event* ev);

	bool Init();
	void FIFOAdd(u8 v);

	void Register(EventHandler handler);
	void Unregister(EventHandler handler);
}