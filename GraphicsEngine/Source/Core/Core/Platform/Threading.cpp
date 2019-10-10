
#include "Threading.h"
#include <algorithm>
#include <typeinfo>

namespace Threading
{
	Event::Event()
	{
#ifdef PLATFORM_WINDOWS
		Handle = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
	}

	Event::~Event()
	{
#ifdef PLATFORM_WINDOWS
		if (Handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(Handle);
		}
#endif
	}

	void Event::Signal()
	{
#ifdef PLATFORM_WINDOWS
		SetEvent(Handle);
#endif
	}

	bool Event::WaitForSignal(int milliseconds)
	{
#ifdef PLATFORM_WINDOWS
		if (milliseconds < 0)
		{
			milliseconds = INFINITE;
		}
		return WaitForSingleObjectEx(Handle, milliseconds, TRUE) == WAIT_OBJECT_0;
#endif
		return false;
	}

	Thread::~Thread()
	{
		if (Handle != NULL)
		{
			RequestToExit();
			WaitForThreadToFinish(-1);
		}
#ifdef PLATFORM_WINDOWS
		CloseHandle(Handle);
#endif
	}
#ifdef PLATFORM_WINDOWS
	DWORD WINAPI Thread::ThreadMain(void *threadAsVoidPtr)
	{
		Thread *thread = (Thread *)threadAsVoidPtr;
		while (!thread->IsRequestedToExit())
		{
			thread->JobReady.WaitForSignal(-1);
			if (thread->IsRequestedToExit())
			{
				thread->JobDone.Signal();//we have quit 
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
#endif
	TaskGraph::TaskGraph(int Count)
	{
		ThreadCount = Count;
		Threads = new Thread*[ThreadCount];
		for (int i = 0; i < ThreadCount; i++)
		{
			Threads[i] = new Thread(i);
		}
	}

	void TaskGraph::Shutdown()
	{
		for (int i = 0; i < ThreadCount; i++)
		{
			Threads[i]->RequestToExit();
			Threads[i]->JobReady.Signal();
			//Threads[i]->WaitForFunctionCompletion();
			SafeDelete(Threads[i]);
		}
	}

	void TaskGraph::RunTaskOnGraph(std::function<void(int)> function, int threadstouse)
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

	int TaskGraph::GetThreadCount() const
	{
		return ThreadCount;
	}
}