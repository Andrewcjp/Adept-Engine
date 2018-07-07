#include "stdafx.h"
#include "WindowsHelper.h"

void WindowsHelpers::DisplayMessageBox(std::string title, std::string message)
{
	MessageBox(
		NULL,
		StringUtils::ConvertStringToWide(message).c_str(),
		StringUtils::ConvertStringToWide(title).c_str(),
		MB_ICONERROR
	);
}

bool WindowsHelpers::DisplayOpenFileDialog(std::string StartDir, std::string Filter, std::string& outData)
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

bool WindowsHelpers::DisplaySaveFileDialog(std::string StartDir, std::string Filter,std::string Extenstion, std::string& outData)
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

bool WindowsHelpers::CopyFileToTarget(std::string Target, std::string Dest)
{
	return CopyFile(StringUtils::ConvertStringToWide(Target).c_str(), StringUtils::ConvertStringToWide(Dest).c_str(), false);
}
