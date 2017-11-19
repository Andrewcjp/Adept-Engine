#pragma once
#include "StringUtil.h"
namespace WindowsHelpers
{
	void DisplayMessageBox(std::string title, std::string message);
	bool DisplayOpenFileDialog(std::string StartDir, std::string Filter, std::string & outData);
	bool DisplaySaveFileDialog(std::string StartDir, std::string Filter, std::string & outData);
}
//int DisplayConfirmSaveAsMessageBox()
//{
//	int msgboxID = MessageBox(
//		NULL,
//		L"temp.txt already exists.\nDo you want to replace it?",
//		L"Confirm Save As",
//		MB_ICONEXCLAMATION | MB_YESNO
//	);
//
//	if (msgboxID == IDYES)
//	{
//
//	}
// = "AnyFile\0*.*\0"
//	return msgboxID;
//}
//usage example
//std::string target;
//using namespace std::string_literals;
//if (WindowsHelpers::DisplayOpenFileDialog(Engine::GetRootDir(), "TextFiles\0*.txt\0"s, target))
//{
//	WindowsHelpers::DisplayMessageBox("", target);
//}