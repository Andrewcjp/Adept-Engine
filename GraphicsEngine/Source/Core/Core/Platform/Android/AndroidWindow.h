#pragma once
#ifdef PLATFORM_ANDROID
#include "..\Generic\GenericWindow.h"
class AndroidWindow : public GenericWindow
{
public:
	AndroidWindow();
	~AndroidWindow();
};
typedef AndroidWindow PlatformWindow;
#endif