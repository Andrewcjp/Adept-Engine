#include "Stdafx.h"
#include "WindowsApplication.h"
#include "../Core/Utils/FileUtils.h"
#include "../Core/Utils/StringUtil.h"


WindowsApplication::WindowsApplication()
{}


WindowsApplication::~WindowsApplication()
{}

bool WindowsApplication::ExecuteHostScript(std::string Filename,std::string Args)
{
	if (FileUtils::exists_test3(Filename))
	{
		//system((Filename + Args).c_str());
		const std::wstring wideFilename = StringUtils::ConvertStringToWide(Filename);
		const std::wstring wideArgs = StringUtils::ConvertStringToWide(Args);
		SHELLEXECUTEINFO ShExecInfo = { 0 };
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = GetDesktopWindow();
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = wideFilename.c_str();
		ShExecInfo.lpParameters = wideArgs.c_str();
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_HIDE;
		ShExecInfo.hInstApp = NULL;
		if (!ShellExecuteEx(&ShExecInfo))
		{
			return false;
		}
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
		return true;
	}
	return false;
}
