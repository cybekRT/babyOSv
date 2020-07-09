#include"LinkedList.h"
#include"Interrupt.h"

namespace Keyboard
{
	LinkedList<u8> keys;

	__attribute__((interrupt))
	void ISR_Keyboard(void*)
	{
		u8 key = HAL_In(0x60);
		keys.PushBack(key);

		//PutString("Keys: "); PutHex(keys.Size()); PutString("\n");

		Interrupt::AckIRQ();
	}

	bool Init()
	{
		Interrupt::Register(Interrupt::IRQ2INT(Interrupt::IRQ_KEYBOARD), ISR_Keyboard);

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
}