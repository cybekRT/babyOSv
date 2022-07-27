#include"Containers/List.hpp"
#include"Thread.hpp"

class Signal
{
	protected:
		bool raised;
		List<Thread::Thread*> waitingThreads;

	public:
		Signal();
		~Signal();

		void Wait();
		void Raise();
		void Reset();
};
