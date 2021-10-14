#include"Thread.hpp"
#include"Memory.h"
#include"Container/Array.h"
#include"Container/LinkedList.h"
#include"Interrupt.h"
#include"Timer.h"

extern bool shellReady;
int strlen(const char* str);

namespace Thread
{
	Thread* currentThread = nullptr;

	Thread* idleThread = nullptr;
	Array<Thread*> threads;

	__attribute((naked, noreturn)) void _NextThread(void*);

	struct SignalInfo
	{
		Signal signal;
		Timer::Time raisedTime;
		Timer::Time timeout;
	};

	LinkedList<SignalInfo> raisedSignals;
	u32 lastThreadId = 0;

	struct Thread2Signal
	{
		Thread* thread;
		Signal* signal;

		Timer::Time sleepTime;
		Timer::Time timeout;
	};

	LinkedList<Thread2Signal> waitingThreads;

	int Idle(void*)
	{
		for(;;)
		{
			Interrupt::Disable();

			//Terminal::Print("idle,");

			while(!raisedSignals.IsEmpty())
			{
				SignalInfo sigInfo = raisedSignals.PopFront();
				Signal sig = sigInfo.signal;

				auto t2s = waitingThreads.data;
				while(t2s)
				{
					bool sigMatch = t2s->value.signal->type == sig.type && t2s->value.signal->addr == sig.addr;
					bool timeout = (t2s->value.timeout != (Timer::Time)-1) && (Timer::GetTicks() >= t2s->value.sleepTime + t2s->value.timeout);

					if(sigMatch || timeout)
					{
						if(sigMatch)
							(*t2s->value.signal) = sig;
						else if(timeout)
							(*t2s->value.signal) = Signal { .type = Signal::Type::Timeout, .addr = 0 };

						t2s->value.thread->state = State::Running;
						threads.PushBack(t2s->value.thread);
						// Print("Thread \"%s\" is waking up (signal): %s\n", currentThread->name, t2s->value.thread->name);

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

					// Print("Thread \"%s\" is wakeing up: %s\n", t2s->value.thread->name, "timeout");

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

			Interrupt::Enable();
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

		//Create(&awakerThread, (u8*)"Awaker", Awaker);
		Create(&idleThread, (u8*)"Idle", Idle);

		//Start(awakerThread);

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
		//Print("Switch task...\n");
		__asm("int $255");
		//Print("Return~!\n");
	}

	__attribute((naked, noreturn)) void _NextThread(void*)
	{
		if(currentThread == nullptr || currentThread->state == State::Unstoppable)// || threads.Size() == 0)
		{
			//Print("No... %s\n", currentThread->name);
			__asm("iret");
			//return;
			//__asm("ret");
		}
		//Print("Prev thread: %s (%d)\n", currentThread->name, threads.Size());

		__asm("cli");

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
			threads.PushBack(currentThread);

		if(threads.Size() == 0)
			currentThread = idleThread;
		else
			currentThread = threads.PopFront();

		currentThread->state = State::Running;

		//Print("Next thread: %s (%d)\n", currentThread->name, threads.Size());

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
		(*thread) = (Thread*)Memory::Alloc(sizeof(Thread));

		(*thread)->process = nullptr;
		(*thread)->id = (++lastThreadId);
		memcpy((*thread)->name, name, strlen((char*)name)+1);
		(*thread)->state = State::Running;

		(*thread)->stackSize = 8192;
		(*thread)->stackBottom = Memory::Alloc((*thread)->stackSize);
		(*thread)->stack = (void*)((u8*)(*thread)->stackBottom + (*thread)->stackSize);

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

		//Terminal::Print("Thread %s waiting for signal %d:%d...\n", currentThread->name, signal.type, signal.addr);
		currentThread->state = State::Waiting;
		waitingThreads.PushBack(Thread2Signal { .thread = currentThread, .signal = &signal, .sleepTime = Timer::GetTicks(), .timeout = timeout } );
		// Print("Waiting size: %d\n", waitingThreads.Size());
		// Print("Thread \"%s\" waiting for signal: %d, timeout: %d\n", currentThread->name, signal.type, timeout);

		NextThread();
		Interrupt::Enable();

		// Print("Thread \"%s\" finished waiting: %d\n", currentThread->name, signal.type);
		if(signal.type == Signal::Type::Timeout)
			return Status::Timeout;

		return Status::Success;
	}
}
