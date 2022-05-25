#include"Keyboard.hpp"
#include"Mouse.hpp"
#include"Interrupt.hpp"
#include"HAL.hpp"

#define PS2_REG_CMD 0x64
#define PS2_REG_DAT 0x60
#define PS2_CMD_GET_COMPAQ_STATUS 0x20
#define PS2_CMD_SET_COMPAQ_STATUS 0x60
#define PS2_CMD_ENABLE_AUX 0xA8
#define PS2_CMD_MOUSE_GET_ID 0xF2
#define PS2_CMD_MOUSE_ENABLE_STREAMING 0xF4
#define PS2_CMD_MOUSE_DISABLE_STREAMING 0xF5
#define PS2_CMD_MOUSE_SET_DEFAULTS 0xF6

namespace Mouse
{
	HAL::RegisterRO<PS2::StatusRegister> regStatus(0x64);
	HAL::RegisterWO<u8> regCommand(PS2_REG_CMD);
	HAL::RegisterRW<u8> regData(PS2_REG_DAT);

	void WaitForReadyToRead()
	{
		PS2::StatusRegister tmp;

		tmp = regStatus.Read(); Print("Status: %x", *(u8*)&tmp);
		while(!regStatus.Read().outputBuffer);
		tmp = regStatus.Read(); Print(" -> %x\n", *(u8*)&tmp);
	}

	void WaitForReadyToSend()
	{
		PS2::StatusRegister tmp;

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
		regData.Write(v);
	}

	void SendControllerCmd(u8 v)
	{
		WaitForReadyToSend();
		regCommand.Write(v);
	}

	u8 ReadData()
	{
		WaitForReadyToRead();
		return regData.Read();
	}

	template<class T>
	T ReadRegister()
	{
		T tmp;
		u8* ptr = (u8*)&tmp;
		WaitForReadyToRead();
		*ptr = regData.Read();
		return tmp;
	}

	void SendData(u8 v)
	{
		WaitForReadyToSend();
		regCommand.Write(0xD4);
		WaitForReadyToSend();
		regData.Write(v);
	}

	template<class T>
	void SendRegister(T& v)
	{
		u8* ptr = (u8*)&v;
		WaitForReadyToSend();
		regData.Write(*ptr);
	}

	bool Init()
	{
		Print("Initializing mouse...\n");
		// Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_MOUSE), ISR_Mouse);

		// SendControllerCmd(PS2_CMD_ENABLE_AUX);

		SendControllerCmd(PS2_CMD_GET_COMPAQ_STATUS);
		auto ctrlConf = ReadRegister<PS2::ControllerConfiguration>();

		ctrlConf.mouseIrq = true;
		ctrlConf.disableMouse = false;

		SendControllerCmd(PS2_CMD_SET_COMPAQ_STATUS);
		SendRegister(ctrlConf);
		// while(ReadData() != 0xFA);

		// SendCmd(0xFF);
		// while(ReadData() != 0xAA);
		SendCmd(PS2_CMD_MOUSE_SET_DEFAULTS);
		// ReadData();
		// SendCmd(PS2_CMD_MOUSE_ENABLE_STREAMING);
		// while(ReadData() != 0xFA);

		// SendCmd(PS2_CMD_MOUSE_GET_ID);
		// while(ReadData() != 0xFA);
		// auto mouseId = ReadData();

		// Print("Mouse ID: %d\n", mouseId);

		SendCmd(PS2_CMD_MOUSE_ENABLE_STREAMING);
		ReadData();

		// SendCmd(0xFF);
		// u8 ack = ReadData();

		Print("Mouse initialized~!\n");
		// for(;;) __asm("hlt");

		return true;
	}

	void FIFOAddCmd(u8 v)
	{
		Print("(m) Add cmd: %x\n", v);
	}

	void FIFOAddData(u8 v)
	{
		Print("(m) Add byte: %x\n", v);
	}
}
