#include"Keyboard_map.hpp"
#include"PS2.hpp"

namespace Keyboard
{
	enum class KeyCode;

	enum class KeyType
	{
		Pressed,
		Released
	};

	enum class KeyMod //: u8
	{
		None	= 0,
		Shift	= 1,
		Control	= 2,
		Alt		= 4,
		Meta	= 8,
	};

	struct KeyEvent
	{
		KeyType type;
		KeyCode key;
		KeyMod mod;
		u8 ascii;
	};

	bool Init();

	void FIFOAdd(u8 v);

	void AddEvent(const KeyEvent& event);
	bool ReadEvent(KeyEvent* event);
	bool WaitAndReadEvent(KeyEvent* event);
	bool IsKeyPressed(KeyCode key);
}

inline Keyboard::KeyMod operator|(Keyboard::KeyMod a, Keyboard::KeyMod b)
{
	//ASSERT(sizeof(a) == sizeof(u8), "Mod size mismatch!");
	//ASSERT(sizeof(b) == sizeof(u8), "Mod size mismatch!");

	u8 a_byte = (u8)a;
	u8 b_byte = (u8)b;
	return (Keyboard::KeyMod)(a_byte | b_byte);
}

inline Keyboard::KeyMod& operator|=(Keyboard::KeyMod& a, Keyboard::KeyMod b)
{
	//ASSERT(sizeof(a) == sizeof(u8), "Mod size mismatch!");
	//ASSERT(sizeof(b) == sizeof(u8), "Mod size mismatch!");

	u8 a_byte = (u8)a;
	u8 b_byte = (u8)b;
	return a = (Keyboard::KeyMod)(a_byte | b_byte);
}

/*inline Keyboard::KeyEvent::Mod operator&(Keyboard::KeyEvent::Mod a, Keyboard::KeyEvent::Mod b)
{
	//ASSERT(sizeof(a) == sizeof(u8), "Mod size mismatch!");
	//ASSERT(sizeof(b) == sizeof(u8), "Mod size mismatch!");

	u8 a_byte = (u8)a;
	u8 b_byte = (u8)b;
	return (Keyboard::KeyEvent::Mod)(a_byte & b_byte);
}*/

inline bool operator&(Keyboard::KeyMod a, Keyboard::KeyMod b)
{
	//ASSERT(sizeof(a) == sizeof(u8), "Mod size mismatch!");
	//ASSERT(sizeof(b) == sizeof(u8), "Mod size mismatch!");

	u8 a_byte = (u8)a;
	u8 b_byte = (u8)b;
	return (a_byte & b_byte);
}
