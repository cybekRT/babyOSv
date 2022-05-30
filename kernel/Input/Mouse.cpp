#include"Keyboard.hpp"
#include"Mouse.hpp"
#include"Interrupt.hpp"
#include"HAL.hpp"
#include"Containers/Array.hpp"
#include"Containers/List.hpp"
#include"Thread.hpp"

#include"Timer.hpp"
#include"Mutex.hpp"

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
	List<Event> events;

	// TODO: thread-safe
	Array<EventHandler> handlers;
	Thread::Thread* thread;
	Mutex handlersMutex;

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

	int HandlerTask(void*)
	{
		for(;;)
		{
			Print(".");
			// handlersMutex.Lock();
			for(auto ev : events)
			{
				for(auto h : handlers)
				{
					h(&ev);
				}
			}

			events.Clear();
			// handlersMutex.Unlock();

			// Thread::SetState(thread, Thread::State::Waiting);
			Thread::WaitForSignal(Thread::Signal { .type = Thread::Signal::IRQ, .value = 123 } );
		}

		return 1;
	}

	bool Init()
	{
		Print("Initializing mouse...\n");

		SendControllerCmd(PS2_CMD_GET_COMPAQ_STATUS);
		auto ctrlConf = ReadRegister<PS2::ControllerConfiguration>();

		ctrlConf.mouseIrq = true;
		ctrlConf.disableMouse = false;

		SendControllerCmd(PS2_CMD_SET_COMPAQ_STATUS);
		SendRegister(ctrlConf);
		// ReadData();
		/* while(ReadData() != 0xFA); */

		SendCmd(PS2_CMD_MOUSE_GET_ID);
		ReadData();
		auto mouseId = ReadData();
		Print("Mouse ID: %d\n", mouseId);

		SendCmd(PS2_CMD_MOUSE_SET_DEFAULTS);
		while(ReadData() != 0xFA);

		SendCmd(PS2_CMD_MOUSE_ENABLE_STREAMING);
		while(ReadData() != 0xFA);

		Thread::Create(&thread, (u8*)"Mouse", HandlerTask);
		Thread::Start(thread);

		Print("Mouse initialized~!\n");
		return true;
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

	bool buttonsStates[3] = { 0 };

	void FIFOAdd(u8 v)
	{
		dataBuffer.PushBack(v);

		if(dataBuffer.Size() >= 3)
		{
			MouseStream* ms = (MouseStream*)dataBuffer.Data();

			if(ms->_unused != 1)
			{
				dataBuffer.PopFront();
				return;
			}

			// FIXME
			// handlersMutex.Lock();
			if(ms->buttonLeft != buttonsStates[0])
			{
				auto type = (ms->buttonLeft) ? EventType::ButtonClick : EventType::ButtonRelease;
				events.PushBack( Event { .type = type, .button = Button::Left } );

				buttonsStates[0] = ms->buttonLeft;
			}

			if(ms->buttonMiddle != buttonsStates[1])
			{
				auto type = (ms->buttonMiddle) ? EventType::ButtonClick : EventType::ButtonRelease;
				events.PushBack( Event { .type = type, .button = Button::Middle } );

				buttonsStates[1] = ms->buttonMiddle;
			}

			if(ms->buttonRight != buttonsStates[2])
			{
				auto type = (ms->buttonRight) ? EventType::ButtonClick : EventType::ButtonRelease;
				events.PushBack( Event { .type = type, .button = Button::Right } );

				buttonsStates[2] = ms->buttonRight;
			}

			int movX = (ms->xSign) ? ((s32)ms->xMov | 0xFFFFFF00) : ms->xMov;
			int movY = (ms->ySign) ? ((s32)ms->yMov | 0xFFFFFF00) : ms->yMov;

			if(movX || movY)
			{
				events.PushBack( Event { .type = EventType::Movement, .movement = { .x = movX, .y = movY } } );
			}

			// handlersMutex.Unlock();

			// Print("Mouse: %c%c%c : %d %d\n",
			// 	(ms->buttonLeft) ? 'L' : '_', (ms->buttonMiddle) ? 'M' : '_', (ms->buttonRight) ? 'R' : '_',
			// 	ms->xMov, ms->yMov);

			// Thread::SetState(thread, Thread::State::Running);

			Thread::RaiseSignal(Thread::Signal { .type = Thread::Signal::IRQ, .value = 123 } );

			dataBuffer.Clear();
		}
	}

	void Register(EventHandler handler)
	{
		handlers.PushBack(handler);
	}

	void Unregister(EventHandler handler)
	{
		handlers.Remove(handler);
	}
}
