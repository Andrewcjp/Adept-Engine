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

		// add big number to make bugs apparent where return value is being passed to float
		return Cycles.QuadPart * SecondsPerCycle;// +16777216.0;
	}
	static void Sleep(float Seconds)
	{
		int Milliseconds = (int)(Seconds * 1000.0);
		if (Milliseconds == 0)
		{
			::SwitchToThread();
		}
		::Sleep(Milliseconds);
	}
private:
	static double SecondsPerCycle;
};

