#include"Keyboard.hpp"
#include"Mouse.hpp"
#include"Interrupt.hpp"
#include"HAL.hpp"
#include"Containers/Array.hpp"

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

	Array<u8> dataBuffer(8);

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

		// SendControllerCmd(PS2_CMD_ENABLE_AUX);

		SendControllerCmd(PS2_CMD_GET_COMPAQ_STATUS);
		auto ctrlConf = ReadRegister<PS2::ControllerConfiguration>();

		ctrlConf.mouseIrq = true;
		ctrlConf.disableMouse = false;

		SendControllerCmd(PS2_CMD_SET_COMPAQ_STATUS);
		SendRegister(ctrlConf);
		// ReadData();
		/* while(ReadData() != 0xFA); */

		SendCmd(PS2_CMD_MOUSE_GET_ID);
		// while(ReadData() != 0xFA);
		ReadData();
		auto mouseId = ReadData();
		Print("Mouse ID: %d\n", mouseId);

		// SendCmd(0xFF);
		// u8 ack = ReadData();

		SendCmd(PS2_CMD_MOUSE_SET_DEFAULTS);
		while(ReadData() != 0xFA);

		SendCmd(PS2_CMD_MOUSE_ENABLE_STREAMING);
		while(ReadData() != 0xFA);

		Print("Mouse initialized~!\n");

		return true;
	}

	void FIFOAddCmd(u8 v)
	{
		Print("(m) Add cmd: %x\n", v);
	}

	struct MouseStream
	{
		u8 buttonLeft : 1;
		u8 buttonRight : 1;
		u8 buttonMiddle : 1;
		u8 _unused : 1;
		u8 xSign : 1;
		u8 ySign : 1;
		u8 xOverflow : 1;
		u8 yOverflow : 1;

		u8 xMov;
		u8 yMov;
	} __attribute__((packed));

	void FIFOAddData(u8 v)
	{
		Print("(m) Add byte: %x\n", v);

		dataBuffer.PushBack(v);

		if(dataBuffer.Size() >= 3)
		{
			MouseStream* ms = (MouseStream*)dataBuffer.Data();

			Print("Mouse: %c%c%c : %d %d\n",
				(ms->buttonLeft) ? 'L' : '_', (ms->buttonMiddle) ? 'M' : '_', (ms->buttonRight) ? 'R' : '_',
				ms->xMov, ms->yMov);

				dataBuffer.PopFront();
				dataBuffer.PopFront();
				dataBuffer.PopFront();
		}
	}

	void Test()
	{
		__asm("sti");

		for(;;)
		{
			__asm("hlt");
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
