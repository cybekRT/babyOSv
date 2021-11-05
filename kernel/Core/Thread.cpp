#include"Thread.hpp"
#include"Memory.hpp"
#include"Container/Array.hpp"
#include"Container/LinkedList.hpp"
#include"Interrupt.hpp"
#include"Timer.hpp"

extern bool shellReady;
//int strlen(const char* str);

namespace Thread
{
	Thread* currentThread = nullptr;

	Thread* idleThread = nullptr;
	Container::Array<Thread*> threads;

	__attribute((naked, noreturn)) void _NextThread(void*);

	struct SignalInfo
	{
		Signal signal;
		Timer::Time raisedTime;
		Timer::Time timeout;
	};

	Container::LinkedList<SignalInfo> raisedSignals;
	u32 lastThreadId = 0;

	struct Thread2Signal
	{
		Thread* thread;
		Signal* signal;

		Timer::Time sleepTime;
		Timer::Time timeout;
	};

	Container::LinkedList<Thread2Signal> waitingThreads;

	void UpdateThreadsList()
	{
		__asm("pushf\ncli");

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
		__asm("popf");
	}

	int Idle(void*)
	{
		int yolo = 0;
		for(;;)
		{
			Interrupt::Disable();
			// __asm("pushf\ncli");

			UpdateThreadsList();

			// Print("========== Idle ==========\n");

			yolo++;
			if(yolo >= 30)
			{
				yolo = 0;
				// Terminal::Print("idle,");
			}

			while(!raisedSignals.IsEmpty())
			{
				SignalInfo sigInfo = raisedSignals.PopFront();
				Signal sig = sigInfo.signal;

				auto t2s = waitingThreads.data;
				while(t2s)
				{
					bool sigMatch = t2s->value.signal->type == sig.type && t2s->value.signal->addr == sig.addr;
					bool timeout = (t2s->value.timeout != (Timer::Time)-1) && (Timer::GetTicks() >= t2s->value.sleepTime + t2s->value.timeout);

					/*if(t2s->value.signal->type == Signal::LockObject)
						Print("Sig: %d, Time: %d, %s\n", sigMatch, timeout, t2s->value.thread->name);*/

					if(sigMatch || timeout)
					{
						if(sigMatch)
							(*t2s->value.signal) = sig;
						else if(timeout)
							(*t2s->value.signal) = Signal { .type = Signal::Type::Timeout, .addr = 0 };

						t2s->value.thread->state = State::Running;
						//Print("Waking thread: %s\n", t2s->value.thread->name);
						threads.PushBack(t2s->value.thread);

						/*Print("Running threads:\n");
						for(auto t : threads)
						{
							Print("-- %s\n", t->name);
						}*/

						//Print("Thread \"%s\" is waking up (signal): %s\n", currentThread->name, t2s->value.thread->name);

						auto item = t2s;
						t2s = t2s->next;
						waitingThreads.Remove(item);
						// Print("Ok?\n");
					}
					else
						t2s = t2s->next;
				}
			}

			//if(waitingThreads.Size() > 1)
			//	Print("Waiting size: %d\n", waitingThreads.Size());
			auto t2s = waitingThreads.data;
			// Print("Threads (bef) waiting:\n");
			static bool wasKmain = false;

			// bool haltForever = false;
			while(t2s)
			{
				// if(t2s->value.thread == idleThread)
				// {
				// 	Print("WTFx~!?\n");
				// 	//for(;;);
				// }

				// Print("- %s\n", t2s->value.thread->name);
				bool timeout = (t2s->value.timeout != (Timer::Time)-1) && (Timer::GetTicks() >= t2s->value.sleepTime + t2s->value.timeout);
				// if(!strcmp((char*)t2s->value.thread->name, "kmain"))
				// 	wasKmain = true;

				// if(wasKmain && shellReady && waitingThreads.Size() == 1 && strcmp((char*)t2s->value.thread->name, "kmain"))
				// {
				// 	//for(;;);
				// 	haltForever = true;
				// }

				//	Print("Waiting: %s - %d\n", t2s->value.thread->name, timeout);

				if(timeout)
				{
					/*if(strcmp((char*)t2s->value.thread->name, "kmain"))
						Print("Timeout: %s - %d\n", t2s->value.thread->name, timeout);*/

					//Print("Thread \"%s\" is waking up: %s\n", t2s->value.thread->name, "timeout");

					(*t2s->value.signal) = Signal { .type = Signal::Type::Timeout, .addr = 0 };

					// Print("Thread \"%s\" is waking up (timeout): %s\n", currentThread->name, t2s->value.thread->name);
					t2s->value.thread->state = State::Running;
					threads.PushBack(t2s->value.thread);

					auto item = t2s;
					t2s = t2s->next;
					// Print("Removing item: %s\n", item->value.thread->name);
					waitingThreads.Remove(item);
					// Print("Ok?\n");
				}
				else
					t2s = t2s->next;
			}

			// auto t2sx = waitingThreads.data;
			// Print("Threads (aft) waiting:\n");
			// while(t2sx)
			// {
			// 	Print("- %s\n", t2sx->value.thread->name);
			// 	t2sx = t2sx->next;
			// }

			// Print("Threads running:\n");

			// for(auto t : threads)
			// {
			// 	Print("- %s\n", t->name);
			// }

			// //if(haltForever)
			// //	for(;;);

			Interrupt::Enable();
			// __asm("popf");
			if(threads.Size() > 0)
				NextThread();
			else
				__asm("hlt");
		}
	}

	void CreateKernelStack(Thread* thread)
	{
		u8* newStackBeg = (u8*)0xE0000000;
		const u32 newStackSize = thread->stackSize;

		void *stackPhys = Memory::Physical::Alloc(newStackSize);
		thread->stackBottom = Memory::Logical::Map(stackPhys, newStackBeg - newStackSize, newStackSize);

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
	}

	bool Init()
	{
		Interrupt::Disable();
		__asm("xchg %bx, %bx");

		Interrupt::Register(255, _NextThread);

		Thread* thread = (Thread*)Memory::Alloc(sizeof(Thread));

		thread->process = nullptr;
		thread->id = 0;
		memcpy(thread->name, (void*)"kmain", 6);
		thread->state = State::Running;

		thread->stackSize = 8192*4;
		//thread->stack = Memory::Alloc(thread->stackSize);
		CreateKernelStack(thread);

		//threads.PushBack(thread);
		currentThread = thread;
		currentThread->interruptDisabled = 1;

		//Create(&awakerThread, (u8*)"Awaker", Awaker);
		Create(&idleThread, (u8*)"Idle", Idle);

		Start(idleThread);

		__asm("xchg %bx, %bx");

		Interrupt::Enable();
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
		// auto prevState = currentThread->state;
		// currentThread->state = State::Running;

		//Print("Switch task...\n");
		__asm("int $255");
		//Print("Return~!\n");

		// currentThread->state = prevState;
		Interrupt::Enable();
	}

	__attribute((naked, noreturn)) void _NextThread(void*)
	{
		__asm("cli");

		if(currentThread == nullptr || currentThread->state == State::Unstoppable)// || threads.Size() == 0)
		{
			//Print("No... %s\n", currentThread->name);
			__asm("iret");
			//return;
			//__asm("ret");
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
		if(currentThread->state == State::Running)// && currentThread != idleThread)
		{
			//for(unsigned a = 0; a < threads.Size(); a++)
			/*for(auto t : threads)
			{
//				Thread* t = threads[a];
				if(t->name == currentThread->name)
				{
					Print("Thread: %s, exists: %s\n", t->name, currentThread->name);
					__asm("int $250");
					for(;;);
				}
			}*/

			threads.PushBack(currentThread);
		}
		// else
		// 	Print("Not running: %s, %d\n", currentThread->name, currentThread->state);

		if(threads.Size() == 0)
			currentThread = idleThread;
		else
			currentThread = threads.PopFront();

		currentThread->state = State::Running;

		//Terminal::Print("Switching to: %s\n", currentThread->name);

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
	}

	Status Join(Thread** thread, int* code)
	{
		if(code != nullptr)
			(*code) = -1;

		return Status::Fail;
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
