#include"Container/LinkedList.hpp"
#include"Interrupt.hpp"
#include"HAL.hpp"
#include"Keyboard.hpp"
#include"Thread.hpp"

#include"Keyboard_map.cpp"

using Keyboard::KeyEvent;
typedef Keyboard::KeyCode Key;

namespace Keyboard
{
	Container::LinkedList<KeyEvent> events;

	u8 keysMap[ ((u32)Key::Total + 7) / 8 ];
	HAL::RegisterRO<u8> regStatus(0x64);
	HAL::RegisterWO<u8> regCommand(0x64);
	HAL::RegisterRW<u8> regData(0x60);

	extern KeyCode scanCode2Key[];
	extern KeyInfo keyInfo[];
	
	ISR(Keyboard)
	{
		u8 scanCode = regData.Read();

		if(scanCode == 0xE0)
		{
			// TODO
			Interrupt::AckIRQ();
			return;
		}

		auto type = (scanCode & 0x80) ? KeyType::Released : KeyType::Pressed;
		auto mod = KeyMod::None;
		auto key = KeyCode::None;

		scanCode &= 0x7F;
		key = scanCode2Key[scanCode];

		if(key == Key::None)
		{
			Interrupt::AckIRQ();
			return;
		}

		u32 mapIndex = ((u32)key) / 8;
		u32 mapBit = ((u32)key) % 8;

		if(type == KeyType::Pressed)
			keysMap[mapIndex] |= (1 << mapBit);
		else
			keysMap[mapIndex] &= ~(1 << mapBit);

		if(IsKeyPressed(KeyCode::LeftShift) || IsKeyPressed(Key::RightShift))
			mod |= KeyMod::Shift;

		if(IsKeyPressed(Key::LeftAlt) || IsKeyPressed(Key::RightAlt))
			mod |= KeyMod::Alt;

		if(IsKeyPressed(Key::LeftControl) || IsKeyPressed(Key::RightControl))
			mod |= KeyMod::Control;

		if(IsKeyPressed(Key::LeftMeta) || IsKeyPressed(Key::RightMeta))
			mod |= KeyMod::Meta;

		u8 ascii = (type == KeyType::Released) ? 0 : ( (mod & KeyMod::Shift) && keyInfo[(u32)key].asciiHigh) ? keyInfo[(u32)key].asciiHigh : keyInfo[(u32)key].asciiLow;

		KeyEvent event = {
			.type = type,
			.key = key,
			.mod = mod,
			.ascii = ascii
		};

		// FIXME: debug only
		if(event.type == KeyType::Pressed && event.key == KeyCode::F12)
		{
			__asm("int $0xfe");
		}

		Interrupt::AckIRQ();

		events.PushBack(event);
		Thread::RaiseSignal( { .type = Thread::Signal::IRQ, .value = Interrupt::IRQ_KEYBOARD } );
	}

	bool Init()
	{
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_KEYBOARD), ISR_Keyboard);

		return true;
	}

	bool ReadEvent(KeyEvent* event)
	{
		if(events.IsEmpty())
			return false;

		*event = events.PopFront();

		return true;
	}

	bool WaitAndReadEvent(KeyEvent* event)
	{
		while(events.IsEmpty())
		{
			Thread::WaitForSignal( { .type = Thread::Signal::IRQ, .value = Interrupt::IRQ_KEYBOARD } );
		}

		*event = events.PopFront();
		return true;
	}

	bool IsKeyPressed(KeyCode key)
	{
		u32 mapIndex = ((u32)key) / 8;
		u32 mapBit = ((u32)key) % 8;

		return keysMap[mapIndex] & (1 << mapBit);
	}
}