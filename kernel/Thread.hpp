#include"Timer.h"

class Process;

namespace Thread
{
	enum class Register : u32
	{
		EIP = 0,

		EAX,
		EBX,
		ECX,
		EDX,

		ESI,
		EDI,
		ESP,
		EBP,

		CS,
		DS,
		ES,
		FS,
		GS,

		Count
	};

	struct Signal
	{
		enum Type
		{
			None = 0,
			IRQ,
			Timer,

			Custom = 250,
			Timeout = 255,
		};

		Type type;
		u32 addr;
	};

	enum State
	{
		Created,
		Running,
		Waiting,
		Zombie,

		Unstoppable
	};

	struct Thread
	{
		Process* process;
		u32 id;
		u8 name[32];
		State state;

		void* stackBottom;
		void* stack;
		u32 stackSize;

		u32 regs[(u32)Register::Count];
	};

	bool Init();
	void NextThread();

	Status Create(Thread** thread, void (*entry)(), u8* name);
	Status Join(Thread** thread, int* code);

	void SetState(Thread* thread, State state);
	Status RaiseSignal(Signal signal, Timer::Time timeout = 0);
	Status WaitForSignal(Signal signal, Timer::Time timeout = (Timer::Time)-1);
}