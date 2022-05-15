#include"Timer.hpp"
#include"Interrupt.hpp"
#include"HAL.hpp"

const u8 PIT_PORT_CHANNEL_0	= 0x40;
const u8 PIT_PORT_CHANNEL_1	= 0x41;
const u8 PIT_PORT_CHANNEL_2	= 0x42;
const u8 PIT_PORT_COMMAND	= 0x43;

const u8 PIT_COMMAND_CHANNEL_0	= 0b00 << 6;
const u8 PIT_COMMAND_CHANNEL_1	= 0b01 << 6;
const u8 PIT_COMMAND_CHANNEL_2	= 0b10 << 6;
const u8 PIT_COMMAND_CHANNEL_RB	= 0b11 << 6;

const u8 PIT_COMMAND_AMODE_LATCH_COUNT	= 0b00 << 4;
const u8 PIT_COMMAND_AMODE_LOBYTE	= 0b01 << 4;
const u8 PIT_COMMAND_AMODE_HIBYTE	= 0b10 << 4;
const u8 PIT_COMMAND_AMODE_LOHIBYTE	= 0b11 << 4;

const u8 PIT_COMMAND_OPMODE_0		= 0b11 << 1 ;// (interrupt on terminal count)
const u8 PIT_COMMAND_OPMODE_1		= 0b11 << 1 ;// hardware re-triggerable one-shot)
const u8 PIT_COMMAND_OPMODE_2		= 0b11 << 1 ;// (rate generator)
const u8 PIT_COMMAND_OPMODE_3		= 0b11 << 1 ;// (square wave generator)
const u8 PIT_COMMAND_OPMODE_4		= 0b11 << 1 ;// (software triggered strobe)
const u8 PIT_COMMAND_OPMODE_5		= 0b11 << 1 ;// (hardware triggered strobe)
const u8 PIT_COMMAND_OPMODE_2_ALT	= 0b11 << 1 ;//
const u8 PIT_COMMAND_OPMODE_3_ALT	= 0b11 << 1 ;//

const u8 PIT_COMMAND_BMODE_BINARY	= 0b0 << 0;
const u8 PIT_COMMAND_BMODE_BCD		= 0b1 << 0;

#include"Thread.hpp"

namespace Timer
{
	volatile Time ticks = 0;

	ISR(Timer)
	{
		ticks++;

		Interrupt::AckIRQ();

		// Thread::NextThread();
	}

	ISR(255)
	{
		Thread::NextThread();
	}

	bool Init()
	{
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_TIMER), ISR_Timer);
		//Interrupt::Register(255, ISR_255);

		HAL::Out8(PIT_PORT_COMMAND, PIT_COMMAND_CHANNEL_0 | PIT_COMMAND_AMODE_LOHIBYTE | PIT_COMMAND_OPMODE_3 | PIT_COMMAND_BMODE_BINARY);
		HAL::Out8(PIT_PORT_CHANNEL_0, 0xA9);
		HAL::Out8(PIT_PORT_CHANNEL_0, 0x04);

		return true;
	}

	Time GetTicks()
	{
		return ticks;
	}

	bool tested = false;
	void Delay(Time ms)
	{
		// __asm("pushf\ncli");
		// auto prevState = Thread::GetState(Thread::currentThread);
		//Thread::SetState(Thread::currentThread, Thread::State::Waiting);

		// Thread::WaitForSignal(Thread::Signal { .type = Thread::Signal::Timeout, .addr = 0 }, ms);

		// FIXME: call to delay if interrupts are disabled
		__asm("pushf");
		__asm("sti");

		static Time time = ticks + ms;
		while(ticks < time)
		{
			// __asm("int $0xff");
		}

		// Thread::SetState(Thread::currentThread, prevState);
		__asm("popf");
	}
}