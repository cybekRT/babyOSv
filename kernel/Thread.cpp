#include"Thread.hpp"
#include"Memory.h"
#include"LinkedList.h"

extern const void* kernel_end;
extern const void* org_stack_beg;
extern const void* org_stack_end;
extern const u32 org_stack_size;
int strlen(const char* str);

namespace Thread
{
	Thread* currentThread = nullptr;
	LinkedList<Thread*> threads;

	void CreateKernelStack(Thread* thread)
	{
		u8* newStackBeg = (u8*)0xE0000000;
		const u32 newStackSize = thread->stackSize;

		void *stackPhys = Memory::AllocPhys(newStackSize);
		thread->stack = Memory::Map(stackPhys, newStackBeg - newStackSize, newStackSize);

		u32 currentESP;
		u32 currentEBP;
		__asm("mov %%esp, %0" : "=r"(currentESP));
		__asm("mov %%ebp, %0" : "=r"(currentEBP));
		u32 oldStackBytesUsed = (u32)org_stack_beg - currentESP;
		u32 oldEBPOffset = (u32)org_stack_beg - currentEBP;

		u32 newESP = (u32)newStackBeg - oldStackBytesUsed;
		u32 newEBP = (u32)newStackBeg - oldEBPOffset;

		memcpy((void*)newESP, (void*)currentESP, oldStackBytesUsed);

		// Fix stack frames
		u32* _ebp = (u32*)newEBP;
		while(*_ebp)
		{
			u32 offset = (u32)org_stack_beg - *_ebp;
			*_ebp = (u32)newStackBeg - offset;
			_ebp = (u32*)*_ebp;
		}

		__asm("mov %0, %%esp" : : "r"(newESP));
		__asm("mov %0, %%ebp" : : "r"(newEBP));
	}

	bool Init()
	{
		Thread* thread = (Thread*)Memory::Alloc(sizeof(Thread));

		thread->process = nullptr;
		thread->name[0] = 0;
		thread->stackSize = 8192;
		//thread->stack = Memory::Alloc(thread->stackSize);
		CreateKernelStack(thread);

		threads.PushBack(thread);
		currentThread = thread;

		return true;
	}

	u32 regsDump[(u32)Register::Count];
	__attribute((naked)) void NextThread()
	{
		// Dump old registers
		__asm("mov (%%esp), %0" : "=a"(regsDump[(u32)Register::EIP]));

		__asm("mov %%eax, %0" : "=a"(regsDump[(u32)Register::EAX]));
		__asm("mov %%ebx, %0" : "=a"(regsDump[(u32)Register::EBX]));
		__asm("mov %%ecx, %0" : "=a"(regsDump[(u32)Register::ECX]));
		__asm("mov %%edx, %0" : "=a"(regsDump[(u32)Register::EDX]));

		__asm("mov %%esi, %0" : "=a"(regsDump[(u32)Register::ESI]));
		__asm("mov %%edi, %0" : "=a"(regsDump[(u32)Register::EDI]));
		__asm("mov %%esp, %0" : "=a"(regsDump[(u32)Register::ESP]));
		__asm("mov %%ebp, %0" : "=a"(regsDump[(u32)Register::EBP]));

		__asm("mov %%cs, %0" : "=a"(regsDump[(u32)Register::CS]));
		__asm("mov %%ds, %0" : "=a"(regsDump[(u32)Register::DS]));
		__asm("mov %%es, %0" : "=a"(regsDump[(u32)Register::ES]));
		__asm("mov %%fs, %0" : "=a"(regsDump[(u32)Register::FS]));
		__asm("mov %%gs, %0" : "=a"(regsDump[(u32)Register::GS]));

		// Copy old registers
		for(unsigned a = 0; a < (u32)Register::Count; a++)
		{
			currentThread->regs[a] = regsDump[a];
		}

		// Change thread
		threads.PushBack(currentThread);
		currentThread = threads.PopFront();


	}

	Status Create(Thread** thread, void (*entry)(), u8* name)
	{
		(*thread) = (Thread*)Memory::Alloc(sizeof(Thread));

		(*thread)->process = nullptr;
		memcpy((*thread)->name, name, strlen((char*)name)+1);
		(*thread)->stackSize = 8192;
		(*thread)->stack = Memory::Alloc((*thread)->stackSize);

		for(unsigned a = 0; a < (u32)Register::Count; a++)
		{
			(*thread)->regs[a] = 0;
		}

		(*thread)->regs[(u32)Register::EIP] = (u32)entry;
		(*thread)->regs[(u32)Register::CS] = (u32)0x08;
		(*thread)->regs[(u32)Register::DS] = (u32)0x10;

		threads.PushBack((*thread));
		Terminal::Print("Created thread: %s\n", (*thread)->name);

		return Status::Success;
	}

	Status Join(Thread** thread, int* code)
	{
		if(code != nullptr)
			(*code) = -1;

		return Status::Fail;
	}
}
