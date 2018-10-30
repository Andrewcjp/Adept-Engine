#include "Stdafx.h"
#include "Threading.h"
#include <algorithm>
namespace TD
{
	using namespace Threading;

	Event::Event()
	{
		Handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	Event::~Event()
	{
		if (Handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(Handle);
		}
	}

	void Event::Signal()
	{
		SetEvent(Handle);
	}

	bool Event::WaitForSignal(int milliseconds)
	{
		if (milliseconds < 0)
		{
			milliseconds = INFINITE;
		}
		return WaitForSingleObjectEx(Handle, milliseconds, TRUE) == WAIT_OBJECT_0;
	}

	Thread::~Thread()
	{
		if (Handle != NULL)
		{
			RequestToExit();
			WaitForThreadToFinish(-1);
		}

		CloseHandle(Handle);
	}

	DWORD WINAPI Thread::ThreadMain(void *threadAsVoidPtr)
	{
		Thread *thread = (Thread *)threadAsVoidPtr;
		while (!thread->IsRequestedToExit())
		{
			thread->JobReady.WaitForSignal(-1);
			if (thread->IsRequestedToExit())
			{
				break;
			}

			if (thread->FunctionToRun != nullptr)
			{
				thread->FunctionToRun(thread->ThreadIndex);
			}
			thread->JobDone.Signal();
		}
		return 0;
	}
};

TD::Threading::TaskGraph::TaskGraph(int Count)
{
	ThreadCount = Count;
	Threads = new Thread*[ThreadCount];
	for (int i = 0; i < ThreadCount; i++)
	{
		Threads[i] = new Thread(i);
	}
}
void TD::Threading::TaskGraph::Shutdown()
{
	for (int i = 0; i < ThreadCount; i++)
	{
		Threads[i]->RequestToExit();
		Threads[i]->JobReady.Signal();
		Threads[i]->WaitForFunctionCompletion();
		SafeDelete(Threads[i]);
	}
}
void TD::Threading::TaskGraph::RunTaskOnGraph(std::function<void(int)> function, int threadstouse)
{
	if (threadstouse == 0)
	{
		threadstouse = ThreadCount;
	}
	threadstouse = std::min(threadstouse, ThreadCount);
	for (int i = 0; i < threadstouse; i++)
	{
		Threads[i]->StartFunction(function);
	}

	for (int i = 0; i < threadstouse; i++)
	{
		Threads[i]->WaitForFunctionCompletion();
	}
}

int TD::Threading::TaskGraph::GetThreadCount() const
{
	return ThreadCount;
}
