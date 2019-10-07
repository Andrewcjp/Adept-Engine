#include "Stdafx.h"

void GenericWindow::SetCursorType(GenericWindow::CursorType Type)
{}

void GenericWindow::DestorySplashWindow()
{}

void GenericWindow::TickSplashWindow(int amt, std::string Section)
{}

void GenericWindow::DestroyApplication()
{}

GenericWindow * GenericWindow::GetApplication()
{
	return nullptr;
}

void GenericWindow::Kill(int code)
{}

bool GenericWindow::IsActiveWindow()
{
	return false;
}
