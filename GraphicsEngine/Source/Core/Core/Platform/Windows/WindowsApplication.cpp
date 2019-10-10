#include "WindowsApplication.h"
#ifdef PLATFORM_WINDOWS
#include "Core/Utils/FileUtils.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Platform/Logger.h"
#include "Core/MinWindows.h"
#include <VersionHelpers.h>

#include <comdef.h>
#include <shellapi.h>
#include <commdlg.h>
#include <Shlwapi.h>

double WindowsApplication::SecondsPerCycle = 0.0f;

WindowsApplication::WindowsApplication()
{}


WindowsApplication::~WindowsApplication()
{}

void WindowsApplication::Init()
{
	ValidateWindows();
	InitTiming();
}

void* WindowsApplication::GetDllExport(void* DllHandle, const char* ProcName)
{
	return (void*)::GetProcAddress((HMODULE)DllHandle, (ProcName));
}

void WindowsApplication::FreeDllHandle(void* DllHandle)
{
	// It is okay to call FreeLibrary on 0
	::FreeLibrary((HMODULE)DllHandle);
}


void* WindowsApplication::GetDllHandle(FString Name)
{
	void * Handle = LoadLibrary(Name.ToWideString().c_str());
	if (!Handle)
	{
		//log!
#if _DEBUG
		DWORD LastError = GetLastError();
		HRESULT hr = HRESULT_FROM_WIN32(LastError);
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		Log::LogMessage("DLL \"" + Name.ToSString() + "\" load failed: " + StringUtils::ConvertWideToString(errMsg), Log::Severity::Error);
#endif
	}
	return Handle;
}

int WindowsApplication::ExecuteHostScript(std::string Filename, std::string Args, bool ShowOutput)
{
	return ExecuteHostScript(Filename, Args, "", ShowOutput);
}

int WindowsApplication::ExecuteHostScript(std::string Filename, std::string Args, std::string WorkingDir, bool ShowOutput)
{
	if (FileUtils::File_ExistsTest(Filename))
	{
		const std::wstring wideFilename = StringUtils::ConvertStringToWide(Filename);
		const std::wstring wideArgs = StringUtils::ConvertStringToWide(Args);
		const std::wstring wideworkingArgs = StringUtils::ConvertStringToWide(WorkingDir);
		SHELLEXECUTEINFO ShExecInfo = { 0 };
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = GetDesktopWindow();
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = wideFilename.c_str();
		ShExecInfo.lpParameters = wideArgs.c_str();
		ShExecInfo.lpDirectory = NULL;
		if (!WorkingDir.empty())
		{
			ShExecInfo.lpDirectory = wideworkingArgs.c_str();
		}

		if (ShowOutput)
		{
			ShExecInfo.nShow = SW_SHOW;
		}
		else
		{
			ShExecInfo.nShow = SW_HIDE;
		}
		ShExecInfo.hInstApp = NULL;
		if (!ShellExecuteEx(&ShExecInfo))
		{
			return false;
		}
		ensure(ShExecInfo.hProcess);
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
		int ReturnCode = -1;
		GetExitCodeProcess(ShExecInfo.hProcess, (LPDWORD)&ReturnCode);
		return ReturnCode;
	}
	return -1;
}

void WindowsApplication::InitTiming()
{
	LARGE_INTEGER Frequency;
	(QueryPerformanceFrequency(&Frequency));
	SecondsPerCycle = 1.0 / Frequency.QuadPart;
}

double WindowsApplication::Seconds()
{
	LARGE_INTEGER Cycles;
	QueryPerformanceCounter(&Cycles);
	return Cycles.QuadPart * SecondsPerCycle;
}

void WindowsApplication::Sleep(double Milliseconds)
{
	if (Milliseconds == 0.0f)
	{
		::SwitchToThread();
	}
	::Sleep((int)Milliseconds);
}

void WindowsApplication::DisplayMessageBox(std::string title, std::string message)
{
	MessageBox(
		NULL,
		StringUtils::ConvertStringToWide(message).c_str(),
		StringUtils::ConvertStringToWide(title).c_str(),
		MB_ICONERROR
	);
}

bool WindowsApplication::DisplayOpenFileDialog(std::string StartDir, std::string Filter, std::string& outData)
{
	TCHAR*FileName = new TCHAR[MAX_PATH];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	std::wstring filterst = StringUtils::ConvertStringToWide(Filter);
	ofn.lpstrFilter = filterst.c_str();
	std::wstring t = StringUtils::ConvertStringToWide(StartDir);
	ofn.lpstrInitialDir = t.c_str();
	ofn.lpstrFile = FileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Select a File";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		outData = StringUtils::ConvertWideToString((wchar_t*)(FileName));
		return true;
	}
	return false;
}

bool WindowsApplication::DisplaySaveFileDialog(std::string StartDir, std::string Filter, std::string Extenstion, std::string& outData)
{
	TCHAR*FileName = new TCHAR[MAX_PATH];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	std::wstring filterst = StringUtils::ConvertStringToWide(Filter);
	ofn.lpstrFilter = filterst.c_str();
	std::wstring t = StringUtils::ConvertStringToWide(StartDir);
	ofn.lpstrInitialDir = t.c_str();
	ofn.lpstrFile = FileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Select a File";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	if (GetSaveFileName(&ofn))
	{
		outData = StringUtils::ConvertWideToString((wchar_t*)(FileName));
		if (ofn.nFileExtension == 0)
		{
			outData.append(Extenstion);
		}
		return true;
	}
	return false;
}

bool WindowsApplication::CopyFileToTarget(std::string Target, std::string Dest)
{
	if (CopyFile(StringUtils::ConvertStringToWide(Target).c_str(), StringUtils::ConvertStringToWide(Dest).c_str(), false))
	{
		return true;
	}
	DWORD LastError = GetLastError();
	if (LastError == ERROR_ALREADY_EXISTS)
	{
		return true;
	}
	return false;
}

bool WindowsApplication::TryCreateDirectory(const std::string & name)
{
	DWORD LastError;
	if (CreateDirectory(StringUtils::ConvertStringToWide(name).c_str(), NULL))
	{
		return true;
	}
	else
	{
		LastError = GetLastError();
		if (LastError == ERROR_ALREADY_EXISTS)
		{
			return true;
		}
	}
	return false;
}

bool WindowsApplication::IsDebuggerPresent()
{
	return ::IsDebuggerPresent();
}

std::string WindowsApplication::GetExecutablePath()
{
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
#if !BUILD_PACKAGE 
	PathCombine(buffer, buffer, L"..");
#endif
	std::wstring ws(buffer);
	return std::string(ws.begin(), ws.end());

}

void WindowsApplication::ValidateWindows()
{
	if (!IsWindows8OrGreater())
	{
		MessageBox(NULL, L"You need at least Windows 8", L"Version Not Supported", MB_OK);
		exit(-1);
	}
}
#endif