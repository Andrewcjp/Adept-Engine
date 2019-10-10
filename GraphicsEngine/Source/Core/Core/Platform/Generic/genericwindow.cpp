#include "Stdafx.h"
#include "GenericWindow.h"

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

void GenericWindow::GetDesktopResolution(int & horizontal, int & vertical)
{}

IntPoint GenericWindow::GetMousePos()
{
	return IntPoint();
}

void GenericWindow::SetMousePos(IntPoint & point)
{}

char GenericWindow::GetVirtualKeyAsChar(unsigned int key)
{
	return 0;
}

short GenericWindow::GetCharAsVirtualKey(char c)
{
	return 0;
}

bool GenericWindow::IsKeyDown(short key)
{
	return false;
}
