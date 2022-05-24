#include"Keyboard.hpp"
#include"Mouse.hpp"
#include"Interrupt.hpp"
#include"HAL.hpp"

#define PS2_REG_CMD 0x64
#define PS2_REG_DAT 0x60
#define PS2_CMD_GET_COMPAQ_STATUS 0x20
#define PS2_CMD_SET_COMPAQ_STATUS 0x60
#define PS2_CMD_ENABLE_AUX 0xA8

namespace Mouse
{
	HAL::RegisterRO<Keyboard::PS2_StatusRegister> regStatus(0x64);
	HAL::RegisterWO<u8> regCommand(PS2_REG_CMD);
	HAL::RegisterRW<u8> regData(PS2_REG_DAT);

	ISR(Mouse)
	{
		Print("m");
		Interrupt::AckIRQ();
	}

	void WaitForReadyToRead()
	{
		Keyboard::PS2_StatusRegister tmp;

		tmp = regStatus.Read(); Print("Status: %x", *(u8*)&tmp);
		while(!regStatus.Read().outputBuffer);
		tmp = regStatus.Read(); Print(" -> %x\n", *(u8*)&tmp);
	}

	void WaitForReadyToSend()
	{
		Keyboard::PS2_StatusRegister tmp;

		tmp = regStatus.Read(); Print("Status: %x", *(u8*)&tmp);
		while(regStatus.Read().inputBuffer);
		tmp = regStatus.Read(); Print("-> %x\n", *(u8*)&tmp);
	}

	void SendCmd(u8 v)
	{
		// Keyboard::PS2_StatusRegister tmp;
		// *(u8*)&tmp = 0x00;
		// tmp.inputBuffer = 1;
		// Print("Test: %d\n", *(u8*)&tmp);

		WaitForReadyToSend();
		regCommand.Write(0xD4);
		WaitForReadyToSend();
		regCommand.Write(v);
	}

	void SendControllerCmd(u8 v)
	{
		WaitForReadyToSend();
		regCommand.Write(v);
	}

	void SendData(u8 v)
	{
		WaitForReadyToSend();
		regCommand.Write(0xD4);
		WaitForReadyToSend();
		regData.Write(v);
	}

	bool Init()
	{
		Print("Initializing mouse...\n");
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_MOUSE), ISR_Mouse);

		SendControllerCmd(PS2_CMD_ENABLE_AUX);

		WaitForReadyToSend();
		HAL::Out8(0x64, 0x20);
		WaitForReadyToRead();
		u8 _status=(HAL::In8(0x60) | 2);
		WaitForReadyToSend();
		HAL::Out8(0x64, 0x60);
		WaitForReadyToSend();
		HAL::Out8(0x60, _status);

		// SendControllerCmd(PS2_CMD_GET_COMPAQ_STATUS);
		// WaitForReadyToRead();
		// u8 compaqStatus = regData.Read();
		// compaqStatus |= 2;
		// compaqStatus &= ~(0x20);
		// SendControllerCmd(PS2_CMD_SET_COMPAQ_STATUS);
		// SendData(compaqStatus);

		// SendCmd(0xFF);

		Print("Mouse initialized~!\n");
		for(;;) __asm("hlt");

		return true;
	}
}
