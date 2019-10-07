#pragma once
#ifdef PLATFORM_LINUX
#include "..\Generic\GenericApplication.h"
class LinuxApplication :public  GenericApplication
{

};

typedef LinuxApplication PlatformApplication;
#endif