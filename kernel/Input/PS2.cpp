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

	void Handle()
	{
		auto status = regStatus.Read();
		u8 data = regData.Read();

		Print("Cmd or data: %d (%x)\n", status.isData, *(u8*)&status);
		if(status.auxData)
		{
			Mouse::FIFOAddData(data);
		}
		else
		{
			Keyboard::FIFOAddData(data);
		}

		Interrupt::AckIRQ();
	}

	ISR(Keyboard)
	{
		Print("k");
		Handle();
	}

	ISR(Mouse)
	{
		Print("m");
		Handle();
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
		SendCmd(PS2_CMD_DISABLE_KEYBOARD);
		SendCmd(PS2_CMD_DISABLE_MOUSE);

		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_KEYBOARD), ISR_Keyboard);
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_MOUSE), ISR_Mouse);

		Keyboard::Init();
		Mouse::Init();

		SendCmd(PS2_CMD_ENABLE_KEYBOARD);
		SendCmd(PS2_CMD_ENABLE_MOUSE);
	}
}