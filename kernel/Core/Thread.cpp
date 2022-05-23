#include"Thread.hpp"
#include"Memory.hpp"
#include"Containers/Array.hpp"
#include"Containers/List.hpp"
#include"Interrupt.hpp"
#include"Timer.hpp"
#include"preinit.hpp"

namespace Thread
{
	struct Thread2Signal
	{
		Thread* thread;
		Signal* signal;

		Timer::Time sleepTime;
		Timer::Time timeout;
	};

	struct SignalInfo
	{
		Signal signal;
		Timer::Time raisedTime;
		Timer::Time timeout;
	};

	Thread* currentThread = nullptr;

	Thread* idleThread = nullptr;
	Array<Thread*> threads;
	List<Thread2Signal> waitingThreads;
	List<SignalInfo> raisedSignals;
	u32 lastThreadId = 0;

	__attribute((naked, noreturn)) void _NextThread(void*);

	void UpdateThreadsList()
	{
		Interrupt::Disable();

		u32 ox, oy;
		u8 cb, cf;
		Terminal::GetXY(&ox, &oy);
		Terminal::GetColor(&cf, &cb);

		int y = 1;
		const int x = 55;
		const int padLen = 25;
		Terminal::SetXY(x, y++);
		Terminal::PrintWithPadding(padLen, "====================");
		Terminal::SetXY(x, y++);
		Terminal::PrintWithPadding(padLen, " Running:");
		for(auto t : threads)
		{
			Terminal::SetXY(x, y++);
			Terminal::PrintWithPadding(padLen, "   %s  ", t->name);
		}

		Terminal::SetXY(x, y++);
		Terminal::PrintWithPadding(padLen, " Waiting:");

		for(auto t2s : waitingThreads)
		{
			Terminal::SetXY(x, y++);
			s32 remainingTime = (t2s.timeout != (Timer::Time)-1) ? t2s.sleepTime + t2s.timeout - Timer::GetTicks() : -1;
			Terminal::PrintWithPadding(padLen, "   %s (%d, %d)  ", t2s.thread->name, t2s.signal->type, remainingTime);
		}

		Terminal::SetXY(x, y++);
		Terminal::PrintWithPadding(padLen, "====================");

		Terminal::SetXY(ox, oy);
		Terminal::SetColor(cf, cb);

		Interrupt::Enable();
	}

	int Idle(void*)
	{
		for(;;)
		{
			Interrupt::Disable();

			UpdateThreadsList();

			while(!raisedSignals.IsEmpty())
			{
				SignalInfo sigInfo = raisedSignals.PopFront();
				Signal sig = sigInfo.signal;

				auto t2s = waitingThreads.begin();
				while(t2s)
				{
					bool sigMatch = t2s->signal->type == sig.type && t2s->signal->addr == sig.addr;
					bool timeout = (t2s->timeout != (Timer::Time)-1) && (Timer::GetTicks() >= t2s->sleepTime + t2s->timeout);

					if(sigMatch || timeout)
					{
						if(sigMatch)
							(*t2s->signal) = sig;
						else if(timeout)
							(*t2s->signal) = Signal { .type = Signal::Type::Timeout, .addr = 0 };

						t2s->thread->state = State::Running;
						threads.PushBack(t2s->thread);

						//auto item = t2s;
						//t2s = t2s->next;
						t2s = waitingThreads.RemoveAt(t2s);
					}
					else
						t2s++;
				}
			}

			auto t2s = waitingThreads.begin();
			while(t2s)
			{
				bool timeout = (t2s->timeout != (Timer::Time)-1) && (Timer::GetTicks() >= t2s->sleepTime + t2s->timeout);
				if(timeout)
				{
					(*t2s->signal) = Signal { .type = Signal::Type::Timeout, .addr = 0 };

					t2s->thread->state = State::Running;
					threads.PushBack(t2s->thread);

					// auto item = t2s;
					// t2s = t2s->next;
					t2s = waitingThreads.RemoveAt(t2s);
				}
				else
					t2s++;
			}

			Interrupt::Enable();
			if(threads.Size() > 0)
				NextThread();
			else
				__asm("hlt");
		}
	}

	void CreateKernelStack(Thread* thread)
	{
		u8* newStackBeg = (u8*)0xF0000000;
		const u32 newStackSize = thread->stackSize;

		Print("Req new stack size: %d\n", newStackSize);
		void *stackPhys = Memory::Physical::Alloc(newStackSize);
		Print("Stack phys: %p, mapping...\n", stackPhys);
		thread->stackBottom = Memory::Logical::Map(stackPhys, newStackBeg - newStackSize, newStackSize);

		Print("Copying old stack\n");
		u32 currentESP;
		u32 currentEBP;
		__asm("mov %%esp, %0" : "=r"(currentESP));
		__asm("mov %%ebp, %0" : "=r"(currentEBP));
		u32 oldStackBytesUsed = _org_stack_beg - currentESP;
		u32 oldEBPOffset = _org_stack_beg - currentEBP;

		u32 newESP = (u32)newStackBeg - oldStackBytesUsed;
		u32 newEBP = (u32)newStackBeg - oldEBPOffset;

		thread->stack = (void*)newESP;

		memcpy((void*)newESP, (void*)currentESP, oldStackBytesUsed);

		// Fix stack frames
		u32* _ebp = (u32*)newEBP;
		while(*_ebp)
		{
			u32 offset = _org_stack_beg - *_ebp;
			*_ebp = (u32)newStackBeg - offset;
			_ebp = (u32*)*_ebp;
		}

		__asm("mov %0, %%esp" : : "r"(newESP));
		__asm("mov %0, %%ebp" : : "r"(newEBP));

		Print("Stack copied, finished~!\n");
	}

	bool Init()
	{
		Interrupt::Disable();

		Interrupt::Register(255, _NextThread);

		Print("Thread: Alloc thread struct\n");
		Thread* thread = (Thread*)Memory::Alloc(sizeof(Thread));

		thread->process = nullptr;
		thread->id = 0;
		memcpy(thread->name, (void*)"Init", 4);
		thread->state = State::Running;

		Print("Thread: Alloc stack\n");
		thread->stackSize = 8192*8;
		//thread->stack = Memory::Alloc(thread->stackSize);
		CreateKernelStack(thread);

		//threads.PushBack(thread);
		currentThread = thread;
		currentThread->interruptDisabled = 1;

		Print("Thread: create idle thread\n");
		Create(&idleThread, (u8*)"Idle", Idle);

		Print("Thread: start idle thread\n");
		Start(idleThread);

		// XXX: Thread init enables global interrupts~!
		Print("Thread: enable interrupt\n");
		Interrupt::Enable();
		Print("Thread: finished~\n");
		return true;
	}

	void Push(Thread* thread, u32 v)
	{
		u32* ptr = (u32*)thread->stack;
		ptr--;
		(*ptr) = v;
		thread->stack = (void*)ptr;
	}

	void NextThread()
	{
		Interrupt::Disable();
		__asm("int $255");
		Interrupt::Enable();
	}

	__attribute((naked, noreturn)) void _NextThread(void*)
	{
		__asm("cli");

		if(currentThread == nullptr || currentThread->state == State::Unstoppable || threads.Size() == 0)
		{
			__asm("iret");
		}

		__asm("pushl %eax");
		__asm("pushl %ebx");
		__asm("pushl %ecx");
		__asm("pushl %edx");

		__asm("pushl %esi");
		__asm("pushl %edi");
		__asm("pushl %ebp");

		__asm("pushl %ds");
		__asm("pushl %es");
		__asm("pushl %fs");
		__asm("pushl %gs");

		__asm("mov %%esp, %0" : "=a"(currentThread->stack));

		// Change thread
		if(currentThread->state == State::Running)
		{
			threads.PushBack(currentThread);
		}

		if(threads.Size() == 0)
			currentThread = idleThread;
		else
			currentThread = threads.PopFront();

		currentThread->state = State::Running;

		__asm("mov %0, %%esp" : : "a"(currentThread->stack));

		__asm("popl %gs");
		__asm("popl %fs");
		__asm("popl %es");
		__asm("popl %ds");

		__asm("popl %ebp");
		__asm("popl %edi");
		__asm("popl %esi");

		__asm("popl %edx");
		__asm("popl %ecx");
		__asm("popl %ebx");
		__asm("popl %eax");

		__asm("iret");
	}

	__attribute((naked)) void ThreadStart()
	{
		currentThread->state = State::Running;

		currentThread->returnValue = currentThread->entry(currentThread->entryArgs);

		currentThread->state = State::Unstoppable;
		Print("Thread \"%s\" terminated~! (%d)\n", currentThread->name, currentThread->returnValue);

		currentThread->state = State::Zombie;
		NextThread();
		for(;;);
	}

	Status Create(Thread** thread, u8* name, int (*entry)(void*), void* threadData)
	{
		// __asm("pushf\ncli");
		Interrupt::Disable();
		(*thread) = (Thread*)Memory::Alloc(sizeof(Thread));

		(*thread)->process = nullptr;
		(*thread)->id = (++lastThreadId);
		memcpy((*thread)->name, name, strlen((char*)name)+1);
		(*thread)->state = State::Running;

		(*thread)->stackSize = 8192;
		(*thread)->stackBottom = Memory::Alloc((*thread)->stackSize);
		(*thread)->stack = (void*)((u8*)(*thread)->stackBottom + (*thread)->stackSize);
		(*thread)->interruptDisabled = 0;

		for(unsigned a = 0; a < (u32)Register::Count; a++)
		{
			(*thread)->regs[a] = 0;
		}

		(*thread)->entry = entry;
		(*thread)->entryArgs = threadData;
		(*thread)->regs[(u32)Register::DS] = (u32)0x10;

		EFlags eflags = {
			.carry	= 				0,
			.unused1 = 				0,
			.parity = 				0,
			.unused2 = 				0,
			.auxiliary = 			0,
			.unused3 = 				0,
			.zero = 				0,
			.sign = 				0,
			.trap = 				0,
			.interrupt = 			1,
			.direction = 			0,
			.overflow = 			0,
			.ioPrivLevel = 			0,
			.nestedTask = 			0,
			.unused4 = 				0,
			.resume = 				0,
			.mode8086 = 			0,
			.alignmentCheck = 		0,
			.virtualInt = 			0,
			.virtualIntPending = 	0,
			.id = 					0,
			.unused5 = 				0,
		};

		Push((*thread), *(u32*)&eflags);
		Push((*thread), 0x8);
		Push((*thread), (u32)ThreadStart);

		Push((*thread), 0);
		Push((*thread), 0);
		Push((*thread), 0);
		Push((*thread), 0);
		Push((*thread), 0);
		Push((*thread), 0);
		Push((*thread), 0);

		Push((*thread), 0x10);
		Push((*thread), 0x10);
		Push((*thread), 0x10);
		Push((*thread), 0x10);

		Terminal::Print("Created thread: %s\n", (*thread)->name);

		// __asm("popf");
		Interrupt::Enable();
		return Status::Success;
	}

	Status Start(Thread* thread)
	{
		Interrupt::Disable();
		threads.PushBack(thread);
		Terminal::Print("Started thread: %s\n", thread->name);
		Interrupt::Enable();

		return Status::Success;
	}

	Status Join(Thread** thread, int* code)
	{
		if(code != nullptr)
			(*code) = -1;

		return Status::Undefined;
	}

	State GetState(Thread* thread)
	{
		return thread->state;
	}

	void SetState(Thread* thread, State state)
	{
		if(thread == nullptr)
			thread = currentThread;

		thread->state = state;
	}

	Status RaiseSignal(Signal signal, Timer::Time timeout)
	{
		Interrupt::Disable();
		raisedSignals.PushBack(SignalInfo { .signal = signal, .raisedTime = Timer::GetTicks(), .timeout = timeout });
		Interrupt::Enable();

		return Status::Success;
	}

	Status WaitForSignal(Signal signal, Timer::Time timeout)
	{
		Interrupt::Disable();

		// if(currentThread == idleThread)
		// {
		// 	Print("WTF~!?\n");
		// 	//for(;;);
		// }

		auto prevState = currentThread->state;

		//Terminal::Print("Thread %s waiting for signal %d:%d...\n", currentThread->name, signal.type, signal.addr);
		currentThread->state = State::Waiting;
		waitingThreads.PushBack(Thread2Signal { .thread = currentThread, .signal = &signal, .sleepTime = Timer::GetTicks(), .timeout = timeout } );
		// Print("Waiting size: %d\n", waitingThreads.Size());
		// Print("Thread \"%s\" waiting for signal: %d, timeout: %d\n", currentThread->name, signal.type, timeout);

		NextThread();

		//auto prevState = currentThread->state;
		//SetState(currentThread, prevState);
		currentThread->state = prevState;
		Interrupt::Enable();

		// Print("Thread \"%s\" finished waiting: %d\n", currentThread->name, signal.type);
		if(signal.type == Signal::Type::Timeout)
			return Status::Timeout;

		return Status::Success;
	}
}
