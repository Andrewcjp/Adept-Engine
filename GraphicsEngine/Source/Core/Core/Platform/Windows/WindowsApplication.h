#pragma once
#ifdef PLATFORM_WINDOWS
#include "Core/Platform/Generic/GenericApplication.h"
#include "Core/Types/FString.h"

class WindowsApplication : public GenericApplication
{
public:
	WindowsApplication();
	~WindowsApplication();
	static void Init();
	static void * GetDllExport(void * DllHandle, const char * ProcName);
	static void FreeDllHandle(void * DllHandle);
	static void * GetDllHandle(FString Name);
	static int ExecuteHostScript(std::string Filename, std::string Args, bool ShowOutput = false);
	static int ExecuteHostScript(std::string Filename, std::string Args, std::string WorkingDir, bool ShowOutput = false);

	static double Seconds();
	static void Sleep(double Milliseconds);
	static CORE_API void DisplayMessageBox(std::string title, std::string message);
	static bool DisplayOpenFileDialog(std::string StartDir, std::string Filter, std::string & outData);
	static bool DisplaySaveFileDialog(std::string StartDir, std::string Filter, std::string Extenstion, std::string & outData);
	static bool CopyFileToTarget(std::string Target, std::string Dest);
	CORE_API static bool TryCreateDirectory(const std::string & name);
	CORE_API static bool IsDebuggerPresent();
	static std::string GetExecutablePath();
	CORE_API static EPlatforms::Type GetPlatform();
	static CORE_API void DisplayAssertBox(std::string title, std::string message);
private:
	static double SecondsPerCycle;
	static void ValidateWindows();
	static void InitTiming();
};

typedef WindowsApplication PlatformApplication;
#endif