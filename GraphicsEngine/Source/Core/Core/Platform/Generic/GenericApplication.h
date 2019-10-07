#pragma once
#include <string>
#include "Core/Asserts.h"

class GenericApplication
{
public:
	int ExecuteHostScript(std::string Filename, std::string Args, bool ShowOutput = false)
	{ /*ensureMsgf(false, "Generic Application Called")*/
	};
	static void InitTiming()
	{};
	static double Seconds()
	{};
	static void Sleep(float Milliseconds)
	{};
	CORE_API static int64_t GetFileTimeStamp(const std::string& Path);
	CORE_API static bool CheckFileSrcNewer(const std::string& srcpath, const std::string dest);
	CORE_API static bool TryCreateDirectory(const std::string & name);
	CORE_API static bool IsDebuggerPresent();
	static CORE_API void DisplayMessageBox(std::string title, std::string message);
	static bool DisplayOpenFileDialog(std::string StartDir, std::string Filter, std::string & outData);
	static bool DisplaySaveFileDialog(std::string StartDir, std::string Filter, std::string Extenstion, std::string & outData);
	static bool CopyFileToTarget(std::string Target, std::string Dest);
	static void Init();
};

