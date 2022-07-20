#pragma once

#include"Timer.hpp"
#include"Status.hpp"
#include"Containers/String.hpp"
#include"Containers/List.hpp"

class Process;

namespace Thread
{
	enum class Register : u32
	{
		//EIP = 0,

		EAX,
		EBX,
		ECX,
		EDX,

		ESI,
		EDI,
		//ESP,
		EBP,

		//CS,
		DS,
		ES,
		FS,
		GS,

		Count
	};

	struct EFlags
	{
		u32 carry				: 1;
		u32 unused1				: 1;
		u32 parity				: 1;
		u32 unused2				: 1;
		u32 auxiliary			: 1;
		u32 unused3				: 1;
		u32 zero				: 1;
		u32 sign				: 1;
		u32 trap				: 1;
		u32 interrupt			: 1;
		u32 direction			: 1;
		u32 overflow			: 1;
		u32 ioPrivLevel			: 2;
		u32 nestedTask			: 1;
		u32 unused4				: 1;
		u32 resume				: 1;
		u32 mode8086			: 1;
		u32 alignmentCheck		: 1;
		u32 virtualInt			: 1;
		u32 virtualIntPending	: 1;
		u32 id					: 1;
		u32 unused5				: 10;
	} __attribute__((packed));

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
		char name[32];
		State state;

		void* stackBottom;
		void* stack;
		u32 stackSize;

		u32 regs[(u32)Register::Count];
		int (*entry)(void*);
		void* entryArgs;

		u32 interruptDisabled;
		u32 returnValue;

		List<Thread*> deathQueue;
	};

	extern Thread* currentThread;

	void UpdateThreadsList();

	bool Init();
	void NextThread();

	Status Create(Thread** thread, String name, int (*entry)(void*), void* threadData = nullptr);
	Status Start(Thread* thread);
	Status Pause(Thread* thread);
	Status Join(Thread* thread, int* code);
	Status Kill(Thread* thread);

	Status Lock();
	Status Unlock();
	Status SwitchTo(Thread* thread);

	State GetState(Thread* thread);
	void SetState(Thread* thread, State state);
}