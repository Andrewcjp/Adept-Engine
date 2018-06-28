#pragma once
#include "GenericApplication.h"
class WindowsApplication : public GenericApplication
{
public:
	WindowsApplication();
	~WindowsApplication();
	static bool ExecuteHostScript(std::string Filename, std::string Args);
	static void InitTiming()
	{
		LARGE_INTEGER Frequency;
		(QueryPerformanceFrequency(&Frequency));
		SecondsPerCycle = 1.0 / Frequency.QuadPart;
	}
	static double Seconds()
	{
		LARGE_INTEGER Cycles;
		QueryPerformanceCounter(&Cycles);
		return Cycles.QuadPart * SecondsPerCycle;
	}
	static void Sleep(float Milliseconds)
	{
		if (Milliseconds < 0.5f)
		{
			::SwitchToThread();
		}
		::Sleep((int)Milliseconds);
	}
private:
	static double SecondsPerCycle;
};

