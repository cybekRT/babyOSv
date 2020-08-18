#include"LinkedList.h"
#include"Interrupt.h"
#include"HAL.h"
#include"Keyboard.h"
#include"Keyboard_map.h"

using Keyboard::KeyEvent;
typedef Keyboard::KeyCode Key;

namespace Keyboard
{
	LinkedList<KeyEvent> events;

	u8 keysMap[ ((u32)Key::Total + 7) / 8 ];

	extern KeyCode scanCode2Key[];
	extern KeyInfo keyInfo[];
	
	__attribute__((interrupt))
	void ISR_Keyboard(void*)
	{
		u8 scanCode = HAL::In(0x60);

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

		events.PushBack(event);

		Interrupt::AckIRQ();
	}

	bool Init()
	{
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_KEYBOARD), ISR_Keyboard);

		/*SendCommand(0x20);
		u8 resp = ReadCommandResponse();

		resp &= ~(1 << 6);

		SendCommand(0x60);
		SendCommand(resp);

		SendCommand(0xff);*/

		/*SendCommand(0xF5);
		resp = ReadCommandResponse();
		Print("Status: %x\n", resp);

		SendCommand(0xF2);
		resp = ReadCommandResponse();
		Print("Status: %x\n", resp);

		resp = ReadCommandResponse();
		Print("Status: %x\n", resp);
		resp = ReadCommandResponse();
		Print("Status: %x\n", resp);

		HALT;*/

		return true;
	}

	bool ReadEvent(KeyEvent* event)
	{
		if(events.IsEmpty())
			return false;

		*event = events.PopFront();

		return true;
	}

	bool IsKeyPressed(KeyCode key)
	{
		u32 mapIndex = ((u32)key) / 8;
		u32 mapBit = ((u32)key) % 8;

		return keysMap[mapIndex] & (1 << mapBit);
	}

	Keyboard::PS2_StatusRegister ReadStatus()
	{
		Keyboard::PS2_StatusRegister reg;
		*(u8*)&reg = HAL::In(0x64);

		return reg;
	}

	void SendCommand(u8 cmd)
	{
		PS2_StatusRegister status;
		do
		{
			status = ReadStatus();
		} while(status.inputBuffer);

		HAL::Out(0x64, cmd);
	}

	u8 ReadCommandResponse()
	{
		PS2_StatusRegister status;
		do
		{
			status = ReadStatus();
		} while(!status.outputBuffer);

		return HAL::In(0x64);
	}
}