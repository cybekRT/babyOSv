#include"Signal.hpp"
#include"Interrupt.hpp"

Signal::Signal() : raised(false)
{

}

Signal::~Signal()
{

}

void Signal::Wait()
{
	Interrupt::Disable();
	if(raised)
	{
		raised = false;
		Interrupt::Enable();
		return;
	}

	waitingThreads.PushBack(Thread::currentThread);
	Thread::SetState(nullptr, Thread::State::Waiting);

	Interrupt::Enable();
	Thread::NextThread();
}

void Signal::Raise()
{
	Interrupt::Disable();
	raised = true;

	if(!waitingThreads.IsEmpty())
	{
		auto thread = waitingThreads.PopFront();
		Thread::SetState(thread, Thread::State::Running);

		raised = false;
	}

	Interrupt::Enable();
}

void Signal::Reset()
{
	Interrupt::Disable();
	raised = false;
	Interrupt::Enable();
}
