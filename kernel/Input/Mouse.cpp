#include"Keyboard.hpp"
#include"Mouse.hpp"
#include"Interrupt.hpp"
#include"HAL.hpp"

#include"Timer.hpp"

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

		// tmp = regStatus.Read(); Print("Status: %x", *(u8*)&tmp);
		while(!regStatus.Read().outputBuffer);
		// tmp = regStatus.Read(); Print(" -> %x\n", *(u8*)&tmp);
	}

	void WaitForReadyToSend()
	{
		PS2::StatusRegister tmp;

		// tmp = regStatus.Read(); Print("Status: %x", *(u8*)&tmp);
		while(regStatus.Read().inputBuffer);
		// tmp = regStatus.Read(); Print("-> %x\n", *(u8*)&tmp);
	}

	void SendCmd(u8 v)
	{
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
		// __asm("cli");
		// for(;;)
		// {
		// 	auto v = HAL::In8(0x64);
		// 	Print("Status: %x\n", v);
		// 	if(v & 1)
		// 		break;
		// }

		// return HAL::In8(0x60);
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

		SendControllerCmd(PS2_CMD_ENABLE_AUX);

		SendControllerCmd(PS2_CMD_GET_COMPAQ_STATUS);
		auto ctrlConf = ReadRegister<PS2::ControllerConfiguration>();

		ctrlConf.mouseIrq = true;
		ctrlConf.disableMouse = false;

		SendControllerCmd(PS2_CMD_SET_COMPAQ_STATUS);
		SendRegister(ctrlConf);
		// while(ReadData() != 0xFA);

		SendCmd(PS2_CMD_MOUSE_GET_ID);
		while(ReadData() != 0xFA);
		auto mouseId = ReadData();
		Print("Mouse ID: %d\n", mouseId);

		SendCmd(PS2_CMD_MOUSE_SET_DEFAULTS);
		while(ReadData() != 0xFA);

		SendCmd(PS2_CMD_MOUSE_ENABLE_STREAMING);
		while(ReadData() != 0xFA);

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

	ISR(kb)
	{
		Print("(k)\n");
		while(HAL::In8(0x64) & 1)
			HAL::In8(0x60);
		Interrupt::AckIRQ();
	}

	volatile int yolov = 0;
	ISR(yolo)
	{
		Print("(m)\n");
		while(HAL::In8(0x64) & 1)
			yolov = HAL::In8(0x60);
		Interrupt::AckIRQ();
	}

	void Test()
	{
		// Interrupt::Register(Interrupt::INT_IRQ12, ISR_yolo);
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_MOUSE), ISR_yolo);
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_KEYBOARD), ISR_kb);
		SendCmd(PS2_CMD_MOUSE_ENABLE_STREAMING);

		__asm("sti");

		for(;;)
		{
			__asm("hlt");
			Print("%x\n", yolov);
			// Print("%x ", ReadData());
		}

		for(;;);
		SendCmd(0xE9);
		Print("Status: %x %x %x %x\n", ReadData(), ReadData(), ReadData(), ReadData());

		char c = 'D';
		for(;;)
		{
			SendCmd(0xEB);
			Print("%c: %x %x %x %x\r", c++, ReadData(), ReadData(), ReadData(), ReadData());
			if(c == 'Z')
				c = 'A';

			Timer::Delay(500);
		}
	}
}
