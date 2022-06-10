#include"PS2.hpp"
#include"HAL.hpp"
#include"Interrupt.hpp"
#include"Keyboard.hpp"
#include"Mouse.hpp"

namespace PS2
{
	HAL::RegisterRO<StatusRegister> regStatus(0x64);
	HAL::RegisterWO<u8> regCommand(0x64);
	HAL::RegisterRW<u8> regData(0x60);

	bool keyboardEnabled = false;
	bool mouseEnabled = false;

	volatile bool handleData = false;

	void Handle()
	{
		if(handleData)
		{
			auto status = regStatus.Read();
			u8 data = regData.Read();

			if(status.auxData)
			{
				Mouse::FIFOAdd(data);
			}
			else
			{
				Keyboard::FIFOAdd(data);
			}
		}
		else
			Print("(no handle)");
	}

	ISR(Keyboard)
	{
		if(keyboardEnabled)
			Handle();
		Interrupt::AckIRQ();
	}

	ISR(Mouse)
	{
		if(mouseEnabled)
			Handle();
		Interrupt::AckIRQ_PIC2();
		Interrupt::AckIRQ();
	}

	void WaitForReadyToRead()
	{
		while(!regStatus.Read().outputBuffer);
	}

	void WaitForReadyToSend()
	{
		while(regStatus.Read().inputBuffer);
	}

	void SendCmd(u8 v)
	{
		WaitForReadyToSend();
		regCommand.Write(v);
	}

#define PS2_CMD_DISABLE_KEYBOARD 0xAD
#define PS2_CMD_ENABLE_KEYBOARD 0xAE
#define PS2_CMD_DISABLE_MOUSE 0xA7
#define PS2_CMD_ENABLE_MOUSE 0xA8

	bool Init()
	{
		// Interrupt::Disable();
		Print("Initializing PS/2...\n");
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_KEYBOARD), ISR_Keyboard);
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_MOUSE), ISR_Mouse);

		SendCmd(PS2_CMD_DISABLE_KEYBOARD);
		SendCmd(PS2_CMD_DISABLE_MOUSE);

		keyboardEnabled = Keyboard::Init();
		mouseEnabled = Mouse::Init();

		if(keyboardEnabled)
		{
			SendCmd(PS2_CMD_ENABLE_KEYBOARD);
			Print("  Keyboard initialized~!\n");
		}
		if(mouseEnabled)
		{
			SendCmd(PS2_CMD_ENABLE_MOUSE);
			Print("  Mouse initialized~!\n");
		}

		handleData = true;
		// Interrupt::Enable();

		return true;
	}
}