#pragma once
#include <Windows.h>
#include <thread>
namespace Threading
{
	class Event
	{
	private:
		void *Handle;
	public:
		Event();
		~Event();
		void Signal();

		// use -1 for an infinite wait
		bool WaitForSignal(int milliseconds);
	};
	class Thread
	{
	public:
		Thread()
		{
			DWORD id;
			Handle = CreateThread(NULL, 0, &Thread::ThreadMain, this, 0, &id);
		}

		~Thread();

		void RequestToExit()
		{
			/// Signal Main() to finish
			QuitRequested = true;
		}

		bool IsRequestedToExit() const
		{
			return QuitRequested;
		}

		bool WaitForThreadToFinish(int milliseconds)
		{
			return JobDone.WaitForSignal(milliseconds);
		}

		Event JobReady; /// waiting for jobs
		Event JobDone;  /// wait for workers to complete their work
		void StartFunction(std::function <void()> functionToRun)
		{
			FunctionToRun = functionToRun;
			JobReady.Signal();
		}

		void WaitForFunctionCompletion()
		{
			JobDone.WaitForSignal(-1);
		}
	private:
		void *Handle;

		volatile bool QuitRequested = false;
		std::function <void()> FunctionToRun;
		static DWORD WINAPI ThreadMain(void *threadAsVoidPtr);
	};
}