#include"Keyboard_map.hpp"

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

	struct PS2_StatusRegister
	{
		u8 outputBuffer : 1;
		u8 inputBuffer : 1;
		u8 systemFlag : 1;
		u8 isData : 1;
		u8 unused : 2;
		u8 timeoutError : 1;
		u8 parityError : 1;
	} __attribute__((packed));

	bool Init();

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
