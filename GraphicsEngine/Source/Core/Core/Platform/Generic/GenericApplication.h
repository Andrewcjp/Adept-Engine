#pragma once
#include <string>
#include "Core/Asserts.h"

class GenericApplication
{
public:
	static int ExecuteHostScript(std::string Filename, std::string Args, bool ShowOutput = false);;
	static int ExecuteHostScript(std::string Filename, std::string Args, std::string WorkingDir, bool ShowOutput = false);;
	static void InitTiming();;
	static double Seconds();;
	static void Sleep(float Milliseconds);;
	CORE_API static int64_t GetFileTimeStamp(const std::string& Path);
	CORE_API static bool CheckFileSrcNewer(const std::string& srcpath, const std::string dest);
	CORE_API static bool TryCreateDirectory(const std::string & name);
	CORE_API static bool IsDebuggerPresent();
	static CORE_API void DisplayMessageBox(std::string title, std::string message);
	static bool DisplayOpenFileDialog(std::string StartDir, std::string Filter, std::string & outData);
	static bool DisplaySaveFileDialog(std::string StartDir, std::string Filter, std::string Extenstion, std::string & outData);
	static bool CopyFileToTarget(std::string Target, std::string Dest);
	static void Init();
	static std::string GetExecutablePath() { return ""; };

	static void * GetDllExport(void * DllHandle, const char * ProcName);
	static void FreeDllHandle(void * DllHandle);
	static void * GetDllHandle(FString Name);
};

