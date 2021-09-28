#include"Thread.hpp"
#include"Memory.h"
#include"Container/Array.h"
#include"Container/LinkedList.h"
#include"Interrupt.h"
#include"Timer.h"

int strlen(const char* str);

namespace Thread
{
	Thread* currentThread = nullptr;
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

	int Awaker(void*)
	{
		for(;;)
		{
			//while(!raisedSignals.IsEmpty())
			for(unsigned a = 0; a < raisedSignals.Size(); a++)
			{
				SignalInfo sigInfo = raisedSignals.PopFront();
				Signal sig = sigInfo.signal;

				/*if(Timer::GetTicks() < sigInfo.raisedTime + sigInfo.timeout)
				{
					Terminal::Print("== %d %d %d ==\n", (u32)Timer::GetTicks(), (u32)sigInfo.raisedTime, (u32)sigInfo.timeout);
					//raisedSignals.PushBack(sigInfo);
				}*/

				//Terminal::Print("Received signal: %u : %u\n", sig.type, sig.addr);

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

						//Terminal::Print("Waking thread: %s\n", t2s->value.thread->name);

						t2s->value.thread->state = State::Running;
						threads.PushBack(t2s->value.thread);

						auto item = t2s;
						t2s = t2s->next;
						waitingThreads.Remove(item);
					}
					else
						t2s = t2s->next;
				}
			}

			auto t2s = waitingThreads.data;
			while(t2s)
			{
				bool timeout = (t2s->value.timeout != (Timer::Time)-1) && (Timer::GetTicks() >= t2s->value.sleepTime + t2s->value.timeout);

				if(timeout)
				{
					(*t2s->value.signal) = Signal { .type = Signal::Type::Timeout, .addr = 0 };

					//Terminal::Print("Waking thread by timeout: %s\n", t2s->value.thread->name);

					t2s->value.thread->state = State::Running;
					threads.PushBack(t2s->value.thread);

					auto item = t2s;
					t2s = t2s->next;
					waitingThreads.Remove(item);
				}
				else
					t2s = t2s->next;
			}

			NextThread();
		}
	}

	void CreateKernelStack(Thread* thread)
	{
		u8* newStackBeg = (u8*)0xE0000000;
		const u32 newStackSize = thread->stackSize;

		void *stackPhys = Memory::Physical::Alloc(newStackSize);
		thread->stackBottom = Memory::Logical::Map(stackPhys, newStackBeg - newStackSize, newStackSize);
		//thread->stack = newStackBeg;

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

	Thread* awakerThread;
	bool Init()
	{
		Interrupt::Disable();

		Interrupt::Register(250, _NextThread);

		Thread* thread = (Thread*)Memory::Alloc(sizeof(Thread));

		thread->process = nullptr;
		thread->id = 0;
		memcpy(thread->name, (void*)"kmain", 6);
		thread->state = State::Running;

		thread->stackSize = 8192*4;
		//thread->stack = Memory::Alloc(thread->stackSize);
		CreateKernelStack(thread);

		threads.PushBack(thread);
		currentThread = thread;

		Create(&awakerThread, (u8*)"Awaker", Awaker);

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
		__asm("int $250");
		//Print("Return~!\n");
	}

	__attribute((naked, noreturn)) void _NextThread(void*)
	{
		if(currentThread == nullptr || currentThread->state == State::Unstoppable)// || threads.Size() == 0)
		{
			//Print("No...\n");
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

		Interrupt::Disable();
		threads.PushBack((*thread));
		Terminal::Print("Created thread: %s\n", (*thread)->name);
		Interrupt::Enable();

		return Status::Success;
	}

	Status Join(Thread** thread, int* code)
	{
		if(code != nullptr)
			(*code) = -1;

		return Status::Fail;
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

		Interrupt::Enable();
		NextThread();

		if(signal.type == Signal::Type::Timeout)
			return Status::Timeout;

		return Status::Success;
	}
}
