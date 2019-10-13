#pragma once
#ifdef PLATFORM_LINUX
#include "..\Generic\GenericApplication.h"
class LinuxApplication :public  GenericApplication
{

	static EPlatforms::Type GetPlatform();
};

typedef LinuxApplication PlatformApplication;
#endif