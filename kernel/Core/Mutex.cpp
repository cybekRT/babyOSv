#include"Mutex.hpp"
#include"Thread.hpp"
#include"Interrupt.hpp"

Mutex::Mutex() : count(1)
{

}

Mutex::~Mutex()
{

}

void Mutex::Lock()
{
	//Print("Locking...\n");
	for(;;)
	{
		Interrupt::Disable();
		if(!IsReady())
			Thread::WaitForSignal( Thread::Signal { .type = Thread::Signal::Type::LockObject, .addr = this } );

		bool locked = false;
		__asm(
			"mov		$1, %%eax \n"
			"mov		$0, %%ebx \n"
			"cmpxchgl	%%ebx, %0 \n"
			"jnz		.not_locked \n"
			"movb		$1, %1 \n"
			".not_locked: \n"
		: "=m"(count), "=m"(locked) : : "eax", "ebx");

		Interrupt::Enable();
		//Print("Locked: %d (%s)\n", locked, Thread::currentThread->name);
		if(locked)
			break;
	}

	//Print("Locked...\n");
}

void Mutex::Unlock()
{
	//Print("Unlocking...\n");
	__asm(
		"incl	%0"
		: "=m"(count));
	//Print("Unlocked...\n");

	Thread::RaiseSignal( Thread::Signal { .type = Thread::Signal::Type::LockObject, .addr = this } );
	Thread::NextThread();
}

bool Mutex::IsReady()
{
	return (count > 0);
}
