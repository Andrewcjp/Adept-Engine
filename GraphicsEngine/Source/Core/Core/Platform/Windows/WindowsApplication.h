#pragma once
#include "Core/Platform/Generic/GenericApplication.h"
#include "Core/Types/FString.h"
#include "EngineGlobals.h"
class WindowsApplication : public GenericApplication
{
public:
	WindowsApplication();
	~WindowsApplication();
	static void * GetDllExport(void * DllHandle, const char * ProcName);
	static void FreeDllHandle(void * DllHandle);
	static void * GetDllHandle(FString Name);
	static bool ExecuteHostScript(std::string Filename, std::string Args);
	static void InitTiming();
	static double Seconds();
	static void Sleep(float Milliseconds);
	static __declspec(dllexport) void DisplayMessageBox(std::string title, std::string message);
	static bool DisplayOpenFileDialog(std::string StartDir, std::string Filter, std::string & outData);
	static bool DisplaySaveFileDialog(std::string StartDir, std::string Filter, std::string Extenstion, std::string & outData);
	static bool CopyFileToTarget(std::string Target, std::string Dest);
	__declspec(dllexport) static bool TryCreateDirectory(const std::string & name);
private:
	static double SecondsPerCycle;
};

typedef WindowsApplication PlatformApplication;