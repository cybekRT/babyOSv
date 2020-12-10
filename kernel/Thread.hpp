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

	struct Thread
	{
		Process* process;
		u8 name[32];

		void* stackBottom;
		void* stack;
		u32 stackSize;

		u32 regs[(u32)Register::Count];
	};

	bool Init();
	void NextThread();

	Status Create(Thread** thread, void (*entry)(), u8* name);
	Status Join(Thread** thread, int* code);
}