#include"Keyboard_map.h"

namespace Keyboard
{
	enum class KeyCode;

	struct KeyEvent
	{
		enum class Type
		{
			Pressed,
			Released
		};

		/*enum class Key
		{
			/*None = 0,
			ESC,
			LeftShift,
			RightShift,
			LeftAlt,
			RightAlt,
			LeftMeta,
			RightMeta,
			LeftCtrl,
			RightCtrl,
			Q,
			W,
			E,
			Key_1,
			Key_2,* /

			None = 0,
			Escape,
			Key_1,
			Key_2,
			Key_3,
			Key_4,
			Key_5,
			Key_6,
			Key_7,
			Key_8,
			Key_9,
			Key_0,
			Minus,
			Equal,
			Backspace,
			Tab,
			Q,
			W,
			E,
			R,
			T,
			Y,
			U,
			I,
			O,
			P,
			Enter,
			LeftControl,
			RightControl,
			LeftMeta,
			RightMeta,
			A,
			S,
			D,
			F,
			G,
			H,
			J,
			K,
			L,
			SingleQuote,
			BackTick,
			LeftShift,
			BackSlash,
			Z,
			X,
			C,
			V,
			B,
			N,
			M,
			Comma,
			Period,
			Slash,
			RightShift,
			KPAD_Mul,
			LeftAlt,
			RightAlt,
			Space,
			CapsLock,
			LeftBracket,
			RightBracket,
			Semicolon,
			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
			NumLock,
			ScrollLock,
			KPAD_7,
			KPAD_8,
			KPAD_9,
			KPAD_Minus,
			KPAD_4,
			KPAD_5,
			KPAD_6,
			KPAD_Plus,
			KPAD_1,
			KPAD_2,
			KPAD_3,
			KPAD_0,
			KPAD_Period,

			Total
		};*/

		enum class Mod //: u8
		{
			None	= 0,
			Shift	= 1,
			Control	= 2,
			Alt		= 4,
			Meta	= 8,
		};

		Type type;
		KeyCode key;
		Mod mod;
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

	bool HasData();
	u8 ReadData();

	bool ReadEvent(KeyEvent* event);
	bool IsKeyPressed(KeyCode key);

	PS2_StatusRegister ReadStatus();
	void SendCommand(u8 cmd);
	u8 ReadCommandResponse();
}

inline Keyboard::KeyEvent::Mod operator|(Keyboard::KeyEvent::Mod a, Keyboard::KeyEvent::Mod b)
{
	//ASSERT(sizeof(a) == sizeof(u8), "Mod size mismatch!");
	//ASSERT(sizeof(b) == sizeof(u8), "Mod size mismatch!");

	u8 a_byte = (u8)a;
	u8 b_byte = (u8)b;
	return (Keyboard::KeyEvent::Mod)(a_byte | b_byte);
}

inline Keyboard::KeyEvent::Mod& operator|=(Keyboard::KeyEvent::Mod& a, Keyboard::KeyEvent::Mod b)
{
	//ASSERT(sizeof(a) == sizeof(u8), "Mod size mismatch!");
	//ASSERT(sizeof(b) == sizeof(u8), "Mod size mismatch!");

	u8 a_byte = (u8)a;
	u8 b_byte = (u8)b;
	return a = (Keyboard::KeyEvent::Mod)(a_byte | b_byte);
}

inline Keyboard::KeyEvent::Mod operator&(Keyboard::KeyEvent::Mod a, Keyboard::KeyEvent::Mod b)
{
	//ASSERT(sizeof(a) == sizeof(u8), "Mod size mismatch!");
	//ASSERT(sizeof(b) == sizeof(u8), "Mod size mismatch!");

	u8 a_byte = (u8)a;
	u8 b_byte = (u8)b;
	return (Keyboard::KeyEvent::Mod)(a_byte & b_byte);
}
