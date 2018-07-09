#include "Stdafx.h"
#include "Threading.h"
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
		/// The following is not guaranteed to succeed as the thread may be suspended on a variable
		/// in a derived class, which in our case is Step or Done.
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
			thread->FunctionToRun();
		}
		thread->JobDone.Signal();
	}
	return 0;
}