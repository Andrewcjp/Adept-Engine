#pragma once
#ifdef PLATFORM_LINUX
#include "..\Generic\GenericWindow.h"
class LinuxWindow : public  GenericWindow
{
public:
	LinuxWindow();
	~LinuxWindow();
};

typedef LinuxWindow PlatformWindow;
#endif