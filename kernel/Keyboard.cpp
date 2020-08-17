#include"LinkedList.h"
#include"Interrupt.h"
#include"HAL.h"
#include"Keyboard.h"
#include"Keyboard_map.h"

using Keyboard::KeyEvent;
typedef Keyboard::KeyCode Key;

namespace Keyboard
{
	LinkedList<u8> keys;
	LinkedList<KeyEvent> events;

	u8 keysMap[ ((u32)Key::Total + 7) / 8 ];

	extern KeyCode scanCode2Key[];
	//#include"Keyboard_map.h"

	/*
	KeyEvent::Key scanCode2Key[] = {
		Key::None,	// 0x00
		Key::ESC,	// 0x01
		Key::Key_1,	// 0x02
		Key::Key_2,	// 0x03
		Key::None,	// 0x04
		Key::None,	// 0x05
		Key::None,	// 0x06
		Key::None,	// 0x07
		Key::None,	// 0x08
		Key::None,	// 0x09
		Key::None,	// 0x0A
		Key::None,	// 0x0B
		Key::None,	// 0x0C
		Key::None,	// 0x0D
		Key::None,	// 0x0E
		Key::None,	// 0x0F
		Key::None,	// 0x10
		Key::None,	// 0x11
		Key::None,	// 0x12
		Key::None,	// 0x13
		Key::None,	// 0x14
		Key::None,	// 0x15
		Key::None,	// 0x16
		Key::None,	// 0x17
		Key::None,	// 0x18
		Key::None,	// 0x19
		Key::None,	// 0x1A
		Key::None,	// 0x1B
		Key::None,	// 0x1C
		Key::None,	// 0x1D
		Key::None,	// 0x1E
		Key::None,	// 0x1F
		Key::None,	// 0x20
		Key::None,	// 0x21
		Key::None,	// 0x22
		Key::None,	// 0x23
		Key::None,	// 0x24
		Key::None,	// 0x25
		Key::None,	// 0x26
		Key::None,	// 0x27
		Key::None,	// 0x28
		Key::None,	// 0x29
		Key::None,	// 0x2A
		Key::None,	// 0x2B
		Key::None,	// 0x2C
		Key::None,	// 0x2D
		Key::None,	// 0x2E
		Key::None,	// 0x2F
		Key::None,	// 0x30
		Key::None,	// 0x31
		Key::None,	// 0x32
		Key::None,	// 0x33
		Key::None,	// 0x34
		Key::None,	// 0x35
		Key::None,	// 0x36
		Key::None,	// 0x37
		Key::LeftAlt,	// 0x38
		Key::None,	// 0x39
		Key::None,	// 0x3A
		Key::None,	// 0x3B
		Key::None,	// 0x3C
		Key::None,	// 0x3D
		Key::None,	// 0x3E
		Key::None,	// 0x3F
		Key::None,	// 0x40
		Key::None,	// 0x41
		Key::None,	// 0x42
		Key::None,	// 0x43
		Key::None,	// 0x44
		Key::None,	// 0x45
		Key::None,	// 0x46
		Key::None,	// 0x47
		Key::None,	// 0x48
		Key::None,	// 0x49
		Key::None,	// 0x4A
		Key::None,	// 0x4B
		Key::None,	// 0x4C
		Key::None,	// 0x4D
		Key::None,	// 0x4E
		Key::None,	// 0x4F
		Key::None,	// 0x50
		Key::None,	// 0x51
		Key::None,	// 0x52
		Key::None,	// 0x53
		Key::None,	// 0x54
		Key::None,	// 0x55
		Key::None,	// 0x56
		Key::None,	// 0x57
		Key::None,	// 0x58
		Key::None,	// 0x59
		Key::None,	// 0x5A
		Key::None,	// 0x5B
		Key::None,	// 0x5C
		Key::None,	// 0x5D
		Key::None,	// 0x5E
		Key::None,	// 0x5F
		Key::None,	// 0x60
		Key::None,	// 0x61
		Key::None,	// 0x62
		Key::None,	// 0x63
		Key::None,	// 0x64
		Key::None,	// 0x65
		Key::None,	// 0x66
		Key::None,	// 0x67
		Key::None,	// 0x68
		Key::None,	// 0x69
		Key::None,	// 0x6A
		Key::None,	// 0x6B
		Key::None,	// 0x6C
		Key::None,	// 0x6D
		Key::None,	// 0x6E
		Key::None,	// 0x6F
		Key::None,	// 0x70
		Key::None,	// 0x71
		Key::None,	// 0x72
		Key::None,	// 0x73
		Key::None,	// 0x74
		Key::None,	// 0x75
		Key::None,	// 0x76
		Key::None,	// 0x77
		Key::None,	// 0x78
		Key::None,	// 0x79
		Key::None,	// 0x7A
		Key::None,	// 0x7B
		Key::None,	// 0x7C
		Key::None,	// 0x7D
		Key::None,	// 0x7E
		Key::None,	// 0x7F
	};
	*/	

	__attribute__((interrupt))
	void ISR_Keyboard(void*)
	{
		u8 scanCode = HAL::In(0x60);
		//keys.PushBack(scanCode);

		if(scanCode == 0xE0)
		{
			// TODO
			Interrupt::AckIRQ();
			return;
		}

		auto type = (scanCode & 0x80) ? KeyEvent::Type::Released : KeyEvent::Type::Pressed;
		auto mod = KeyEvent::Mod::None;
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

		if(type == KeyEvent::Type::Pressed)
			keysMap[mapIndex] |= (1 << mapBit);
		else
			keysMap[mapIndex] &= ~(1 << mapBit);

		if(IsKeyPressed(KeyCode::LeftShift) || IsKeyPressed(Key::RightShift))
			mod |= KeyEvent::Mod::Shift;

		if(IsKeyPressed(Key::LeftAlt) || IsKeyPressed(Key::RightAlt))
			mod |= KeyEvent::Mod::Alt;

		if(IsKeyPressed(Key::LeftControl) || IsKeyPressed(Key::RightControl))
			mod |= KeyEvent::Mod::Control;

		if(IsKeyPressed(Key::LeftMeta) || IsKeyPressed(Key::RightMeta))
			mod |= KeyEvent::Mod::Meta;

		KeyEvent event = {
			.type = type,
			.key = key,
			.mod = mod,
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

	bool HasData()
	{
		return !keys.IsEmpty();
	}

	u8 ReadData()
	{
		return keys.PopFront();
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