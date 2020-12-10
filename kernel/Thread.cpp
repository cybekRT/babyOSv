#include"Thread.hpp"
#include"Memory.h"
#include"LinkedList.h"
#include"Interrupt.h"

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
		thread->stackBottom = Memory::Map(stackPhys, newStackBeg - newStackSize, newStackSize);
		//thread->stack = newStackBeg; 

		u32 currentESP;
		u32 currentEBP;
		__asm("mov %%esp, %0" : "=r"(currentESP));
		__asm("mov %%ebp, %0" : "=r"(currentEBP));
		u32 oldStackBytesUsed = (u32)org_stack_beg - currentESP;
		u32 oldEBPOffset = (u32)org_stack_beg - currentEBP;

		u32 newESP = (u32)newStackBeg - oldStackBytesUsed;
		u32 newEBP = (u32)newStackBeg - oldEBPOffset;

		thread->stack = (void*)newESP;

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
		Interrupt::Disable();
		Thread* thread = (Thread*)Memory::Alloc(sizeof(Thread));

		thread->process = nullptr;
		//thread->name[0] = 0;
		memcpy(thread->name, (void*)"kmain", 6);
		thread->stackSize = 8192;
		//thread->stack = Memory::Alloc(thread->stackSize);
		CreateKernelStack(thread);

		threads.PushBack(thread);
		currentThread = thread;

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

	u32 regsDump[(u32)Register::Count];
	__attribute((naked)) void NextThread()
	{
		if(currentThread == nullptr)
			__asm("ret");

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
		threads.PushBack(currentThread);
		currentThread = threads.PopFront();

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

		__asm("ret");
	}

	__attribute((naked)) void ThreadStart()
	{
		Terminal::Print("XD\n");
		__asm("xchg %bx, %bx");
		__asm("iret");
	}

	Status Create(Thread** thread, void (*entry)(), u8* name)
	{
		Interrupt::Disable();
		(*thread) = (Thread*)Memory::Alloc(sizeof(Thread));

		(*thread)->process = nullptr;
		memcpy((*thread)->name, name, strlen((char*)name)+1);
		(*thread)->stackSize = 8192;
		(*thread)->stackBottom = Memory::Alloc((*thread)->stackSize);
		(*thread)->stack = (*thread)->stackBottom + (*thread)->stackSize;

		for(unsigned a = 0; a < (u32)Register::Count; a++)
		{
			(*thread)->regs[a] = 0;
		}

		(*thread)->regs[(u32)Register::EIP] = (u32)entry;
		(*thread)->regs[(u32)Register::CS] = (u32)0x08;
		(*thread)->regs[(u32)Register::DS] = (u32)0x10;

		Push((*thread), 0x0200);
		Push((*thread), 0x8);
		Push((*thread), (u32)entry);
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
}
