#pragma once
#include "GenericApplication.h"
class WindowsApplication : public GenericApplication
{
public:
	WindowsApplication();
	~WindowsApplication();
	static bool ExecuteHostScript(std::string Filename, std::string Args);
};

